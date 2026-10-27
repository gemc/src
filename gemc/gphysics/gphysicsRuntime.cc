#include "gphysicsRuntime.h"
#include <gemc/gbase/gbase.h>

#include "G4AutoLock.hh"
#include "G4BuilderType.hh"
#include "G4MTRunManager.hh"
#include "G4ParticleTable.hh"
#include "G4PhysListFactoryAlt.hh"
#include "G4PhysicsConstructorRegistry.hh"
#include "G4ProcessManager.hh"
#include "G4ProcessVector.hh"
#include "G4RunManager.hh"
#include "G4StateManager.hh"
#include "G4StepLimiterPhysics.hh"
#include "G4SystemOfUnits.hh"
#include "G4UIcmdWithoutParameter.hh"
#include "G4UIdirectory.hh"
#include "G4UImanager.hh"
#include "G4UImessenger.hh"
#include "G4VPhysicsConstructor.hh"
#include "G4VProcess.hh"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <set>
#include <stdexcept>
#include <typeinfo>

namespace {

// Match G4VModularPhysicsList: hadronic constructors require serialized process construction.
G4Mutex constructionMutex = G4MUTEX_INITIALIZER;

bool editableState(std::string& error) {
	if (G4Threading::IsWorkerThread()) {
		error = "Physics changes must be requested on the master thread.";
		return false;
	}
	const auto state = G4StateManager::GetStateManager()->GetCurrentState();
	if (state == G4State_PreInit || state == G4State_Idle) { return true; }
	error = "Physics can only be changed before initialization or between runs.";
	return false;
}

void flushWorkerCommands() {
	if (auto* master = dynamic_cast<G4MTRunManager*>(G4RunManager::GetRunManager())) {
		master->RequestWorkersProcessCommandsStack();
	}
}

template<typename Function>
void forManagers(Function function) {
	std::set<G4ProcessManager*> seen;
	auto* iterator = G4ParticleTable::GetParticleTable()->GetIterator();
	iterator->reset(false);
	while ((*iterator)()) {
		auto* manager = iterator->value()->GetProcessManager();
		if (manager && seen.insert(manager).second) { function(manager); }
	}
}

std::string category(const G4VProcess* process) {
	// Geant4 labels these optical-photon production processes as electromagnetic.
	if (process->GetProcessName() == "Cerenkov" || process->GetProcessName() == "Scintillation") {
		return "Optical";
	}
	switch (process->GetProcessType()) {
		case fTransportation: return "Transportation";
		case fElectromagnetic: return "Electromagnetic";
		case fOptical: return "Optical";
		case fHadronic: return "Hadronic";
		case fDecay: return "Decay";
		case fGeneral: return "General";
		case fParameterisation: return "Parameterisation";
		case fParallel: return "Parallel worlds";
		default: return "Other";
	}
}

} // namespace

class GPhysicsMessenger : public G4UImessenger {
public:
	explicit GPhysicsMessenger(gphysics::RuntimePhysicsList& input) : physics(input) {
		directory = std::make_unique<G4UIdirectory>("/gemc/physics/");
		clear = command("clearProcesses");
		rebuild = command("rebuildProcesses");
		activate = command("applyActivation");
	}
	void SetNewValue(G4UIcommand* input, G4String) override {
		if (input == activate.get()) { physics.applyActivation(); }
		else if (G4Threading::IsWorkerThread()) {
			if (input == clear.get()) { physics.clearProcesses(); }
			if (input == rebuild.get()) { physics.rebuildWorker(); }
		}
	}
private:
	std::unique_ptr<G4UIcmdWithoutParameter> command(const std::string& name) {
		const auto path = "/gemc/physics/" + name;
		auto result = std::make_unique<G4UIcmdWithoutParameter>(path.c_str(), this);
		result->AvailableForStates(G4State_PreInit, G4State_Idle);
		result->SetToBeBroadcasted(true);
		return result;
	}
	gphysics::RuntimePhysicsList& physics;
	std::unique_ptr<G4UIdirectory> directory;
	std::unique_ptr<G4UIcmdWithoutParameter> clear, rebuild, activate;
};

namespace {
G4ThreadLocal GPhysicsMessenger* workerMessenger = nullptr;
}

namespace gphysics {

RuntimePhysicsList::RuntimePhysicsList(const std::string& name) {
	std::string error;
	if (!select(name, error)) { throw std::runtime_error(error); }
	messenger = std::make_unique<GPhysicsMessenger>(*this);
}

RuntimePhysicsList::~RuntimePhysicsList() {
	// The factory lists own their constructors; this vector is only the public modular-list view.
	G4VMPLsubInstanceManager.offset[g4vmplInstanceID].physicsVector->clear();
}

RuntimePhysicsList* RuntimePhysicsList::current() {
	auto* manager = G4RunManager::GetRunManager();
	return manager ? dynamic_cast<RuntimePhysicsList*>(
		const_cast<G4VUserPhysicsList*>(manager->GetUserPhysicsList())) : nullptr;
}

std::vector<std::string> RuntimePhysicsList::referenceLists() {
	g4alt::G4PhysListFactory factory;
	const auto& names = factory.AvailablePhysLists();
	return {names.begin(), names.end()};
}

std::vector<std::string> RuntimePhysicsList::constructors() {
	const auto names = G4PhysicsConstructorRegistry::Instance()->AvailablePhysicsConstructors();
	return {names.begin(), names.end()};
}

bool RuntimePhysicsList::select(const std::string& name, std::string& error) {
	if (!editableState(error)) { return false; }
	std::string normalized;
	std::copy_if(name.begin(), name.end(), std::back_inserter(normalized),
	             [](unsigned char ch) { return !std::isspace(ch); });
	if (normalized == selected) { return true; }
	std::set<std::string> additions;
	for (auto start = normalized.find('+'); start != std::string::npos;) {
		const auto end = normalized.find('+', start + 1);
		const auto constructor = normalized.substr(start + 1, end - start - 1);
		if (constructor.empty() || !additions.insert(constructor).second) {
			error = "Physics constructors must be nonempty and cannot be added twice.";
			return false;
		}
		start = end;
	}
	g4alt::G4PhysListFactory factory;
	if (!factory.IsReferencePhysList(normalized)) {
		error = "Unknown physics list or constructor: " + name;
		return false;
	}
	auto* stateManager = G4StateManager::GetStateManager();
	const auto previousState = stateManager->GetCurrentState();
	if (previousState == G4State_Idle) { flushWorkerCommands(); }
	// Factory constructors register physics in PreInit. Workers are idle throughout this transaction.
	stateManager->SetNewState(G4State_PreInit);
	std::unique_ptr<G4VModularPhysicsList> next(factory.GetReferencePhysList(normalized));
	next->RegisterPhysics(new G4StepLimiterPhysics());
	auto reject = [&](const std::string& message) {
		// List destruction would delete all particles. Retain the unused list until shutdown.
		lists.push_back(std::move(next));
		if (!selected.empty()) {
			// Factory construction changes shared defaults; restore the active configuration's defaults.
			lists.emplace_back(factory.GetReferencePhysList(selected));
		}
		stateManager->SetNewState(previousState);
		error = message;
		return false;
	};
	for (const auto& addition : additions) {
		if (!addition.starts_with("G4")) { continue; } // Preserve the extensible factory's named aliases.
		bool installed = false;
		for (int i = 0; const auto* constructor = next->GetPhysics(i); ++i) {
			installed |= demangle(typeid(*constructor).name()) == addition;
		}
		if (!installed) {
			return reject("Constructor conflicts with the reference list: " + addition +
			              ". Select a reference-list variant or use replacement syntax instead.");
		}
	}
	const auto* em = next->GetPhysicsWithType(bElectromagnetic);
	const std::string identity = em ? std::string(typeid(*em).name()) + em->GetPhysicsName() : "";
	if (previousState == G4State_Idle && identity != electromagneticIdentity) {
		return reject("Changing the electromagnetic constructor after initialization requires restarting GEMC.");
	}
	stateManager->SetNewState(previousState);
	if (previousState == G4State_Idle) {
		broadcast("clearProcesses");
		clearProcesses();
	}
	activeConstructors.clear();
	for (int i = 0; const auto* constructor = next->GetPhysics(i); ++i) {
		activeConstructors.push_back(const_cast<G4VPhysicsConstructor*>(constructor));
	}
	*G4VMPLsubInstanceManager.offset[g4vmplInstanceID].physicsVector = activeConstructors;
	// Before a run manager exists there is no default region; defer region cuts until SetCuts().
	defaultCutValue = productionCut.value_or(next->GetDefaultCutValue());
	activeList = next.get();
	lists.push_back(std::move(next));
	selected = normalized;
	electromagneticIdentity = identity;
	if (previousState == G4State_Idle) {
		SetDefaultCutValue(defaultCutValue);
		ConstructParticle();
		auto* manager = G4RunManager::GetRunManager();
		manager->InitializePhysics();
		manager->PhysicsHasBeenModified();
		broadcast("rebuildProcesses");
		broadcast("applyActivation");
	}
	return true;
}

void RuntimePhysicsList::ConstructParticle() {
	for (auto* constructor : activeConstructors) { constructor->ConstructParticle(); }
}

void RuntimePhysicsList::ConstructProcess() {
	G4AutoLock lock(&constructionMutex);
	// Constructors created after worker startup need fresh thread-local split-class slots.
	const_cast<G4VPCManager&>(G4VPhysicsConstructor::GetSubInstanceManager()).NewSubInstances();
	auto& state = threadState.Get();
	if (!state.initialized) {
		AddTransportation();
		forManagers([&](G4ProcessManager* manager) {
			for (int i = 0; i < manager->GetProcessListLength(); ++i) {
				state.retained.insert((*manager->GetProcessList())[i]);
			}
		});
	} else {
		// New neutral particles, such as opticalphoton, also need the existing transportation process.
		const auto transport = std::find_if(state.retained.begin(), state.retained.end(),
			[](const auto* process) { return process->GetProcessType() == fTransportation; });
		forManagers([&](G4ProcessManager* manager) {
			if (manager->GetProcessListLength() == 0 && transport != state.retained.end()) {
				manager->AddProcess(*transport);
				manager->SetProcessOrderingToFirst(*transport, idxAlongStep);
				manager->SetProcessOrderingToFirst(*transport, idxPostStep);
			}
		});
	}
	for (auto* constructor : activeConstructors) {
		const bool electromagnetic = constructor->GetPhysicsType() == bElectromagnetic;
		if (electromagnetic && state.initialized) { continue; }
		std::set<G4VProcess*> before;
		if (electromagnetic) {
			forManagers([&](G4ProcessManager* manager) {
				for (int i = 0; i < manager->GetProcessListLength(); ++i) {
					before.insert((*manager->GetProcessList())[i]);
				}
			});
		}
		constructor->ConstructProcess();
		if (electromagnetic) {
			state.retainedConstructors.push_back(constructor);
			forManagers([&](G4ProcessManager* manager) {
				for (int i = 0; i < manager->GetProcessListLength(); ++i) {
					auto* process = (*manager->GetProcessList())[i];
					if (!before.contains(process)) { state.retained.insert(process); }
				}
			});
		}
	}
	state.initialized = true;
	applyActivation();
}

void RuntimePhysicsList::SetCuts() {
	activeList->SetCuts();
	if (productionCut) { SetCutsWithDefault(); }
}

void RuntimePhysicsList::InitializeWorker() {
	G4VUserPhysicsList::InitializeWorker();
	workerMessenger = new GPhysicsMessenger(*this);
}

void RuntimePhysicsList::TerminateWorker() {
	for (auto* constructor : activeConstructors) {
		if (constructor->GetPhysicsType() != bElectromagnetic) { constructor->TerminateWorker(); }
	}
	for (auto* constructor : threadState.Get().retainedConstructors) { constructor->TerminateWorker(); }
	G4VUserPhysicsList::TerminateWorker();
	delete workerMessenger;
	workerMessenger = nullptr;
}

void RuntimePhysicsList::clearProcesses() {
	G4AutoLock lock(&constructionMutex);
	std::set<G4VProcess*> processes;
	const auto& retained = threadState.Get().retained;
	std::map<G4ProcessManager*, G4ProcessManager*> replacements;
	forManagers([&](G4ProcessManager* manager) {
		// RemoveProcess leaves "ordered first/last" flags set in Geant4. Fresh managers reset that
		// bookkeeping without replacing the EM process objects cached by G4LossTableManager.
		auto* replacement = new G4ProcessManager(manager->GetParticleType());
		replacement->SetVerboseLevel(manager->GetVerboseLevel());
		replacements.emplace(manager, replacement);
		for (int i = 0; i < manager->GetProcessListLength(); ++i) {
			auto* process = (*manager->GetProcessList())[i];
			if (!retained.contains(process)) { continue; }
			const auto rest = manager->GetProcessOrdering(process, idxAtRest);
			const auto along = manager->GetProcessOrdering(process, idxAlongStep);
			const auto post = manager->GetProcessOrdering(process, idxPostStep);
			replacement->AddProcess(process, rest, along, post);
			for (const auto index : {idxAtRest, idxAlongStep, idxPostStep}) {
				if (manager->GetProcessOrdering(process, index) == 0) {
					replacement->SetProcessOrderingToFirst(process, index);
				}
			}
			replacement->SetProcessActivation(process, manager->GetProcessActivation(i));
		}
		for (int i = manager->GetProcessListLength() - 1; i >= 0; --i) {
			auto* process = (*manager->GetProcessList())[i];
			if (!retained.contains(process)) {
				processes.insert(process);
			}
			manager->RemoveProcess(process);
		}
	});
	// General ions can share a manager. Update every particle alias before deleting each old manager once.
	std::vector<std::pair<G4ParticleDefinition*, G4ProcessManager*>> particles;
	auto* iterator = G4ParticleTable::GetParticleTable()->GetIterator();
	iterator->reset(false);
	while ((*iterator)()) {
		auto* particle = iterator->value();
		if (const auto found = replacements.find(particle->GetProcessManager()); found != replacements.end()) {
			particles.emplace_back(particle, found->second);
		}
	}
	for (const auto& [particle, replacement] : particles) {
		particle->SetProcessManager(replacement);
		if (!G4Threading::IsWorkerThread()) { particle->SetMasterProcessManager(replacement); }
	}
	for (const auto& [oldManager, replacement] : replacements) { delete oldManager; }
	for (auto* process : processes) { delete process; }
	if (G4Threading::IsWorkerThread()) {
		for (auto* constructor : activeConstructors) {
			if (constructor->GetPhysicsType() != bElectromagnetic) { constructor->TerminateWorker(); }
		}
	}
}

void RuntimePhysicsList::rebuildWorker() {
	G4RunManager::GetRunManager()->InitializePhysics();
	G4RunManager::GetRunManager()->PhysicsHasBeenModified();
}

void RuntimePhysicsList::broadcast(const std::string& command) {
	if (!messenger) { return; }
	const int status = G4UImanager::GetUIpointer()->ApplyCommand("/gemc/physics/" + command);
	if (status) { throw std::runtime_error("Could not apply physics command: " + command); }
	flushWorkerCommands();
}

bool RuntimePhysicsList::setProductionCut(double millimeters, std::string& error) {
	if (!editableState(error)) { return false; }
	if (!std::isfinite(millimeters) || millimeters <= 0) {
		error = "Production cut must be a positive length in millimeters.";
		return false;
	}
	if (G4StateManager::GetStateManager()->GetCurrentState() == G4State_Idle) {
		// Preserve ordering with native /run/setCut commands waiting in the worker UI queues.
		flushWorkerCommands();
	}
	productionCut = millimeters * mm;
	defaultCutValue = *productionCut;
	if (G4StateManager::GetStateManager()->GetCurrentState() == G4State_Idle) {
		SetCutsWithDefault();
		G4RunManager::GetRunManager()->PhysicsHasBeenModified();
	}
	return true;
}

std::vector<ProcessState> RuntimePhysicsList::processes() const {
	std::map<std::string, ProcessState> result;
	forManagers([&](G4ProcessManager* manager) {
		for (int i = 0; i < manager->GetProcessListLength(); ++i) {
			auto* process = (*manager->GetProcessList())[i];
			const auto name = process->GetProcessName();
			const bool active = manager->GetProcessActivation(i);
			auto [it, inserted] = result.try_emplace(name, ProcessState{
				name, category(process), active, false,
				process->GetProcessType() != fTransportation, {}});
			if (!inserted && active != it->second.active) { it->second.mixed = true; }
			it->second.particles.push_back(manager->GetParticleType()->GetParticleName());
		}
	});
	std::vector<ProcessState> states;
	for (auto& [name, state] : result) {
		std::sort(state.particles.begin(), state.particles.end());
		state.particles.erase(std::unique(state.particles.begin(), state.particles.end()), state.particles.end());
		states.push_back(std::move(state));
	}
	return states;
}

bool RuntimePhysicsList::setProcessEnabled(const std::string& name, bool enabled, std::string& error) {
	if (!editableState(error)) { return false; }
	const auto states = processes();
	const auto found = std::find_if(states.begin(), states.end(),
	                               [&](const auto& state) { return state.name == name; });
	if (found == states.end() || !found->editable) {
		error = "Process is unavailable or required for transportation: " + name;
		return false;
	}
	activation[name] = enabled;
	broadcast("applyActivation");
	G4RunManager::GetRunManager()->PhysicsHasBeenModified();
	return true;
}

void RuntimePhysicsList::applyActivation() {
	forManagers([&](G4ProcessManager* manager) {
		for (int i = 0; i < manager->GetProcessListLength(); ++i) {
			const auto* process = (*manager->GetProcessList())[i];
			const auto choice = activation.find(process->GetProcessName());
			if (choice != activation.end() && process->GetProcessType() != fTransportation) {
				manager->SetProcessActivation(i, choice->second);
			}
		}
	});
}

} // namespace gphysics
