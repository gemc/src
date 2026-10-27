#include "gemc_options.h"
#include <gemc/gdetector/gdetectorConstruction.h>
#include <gemc/gphysics/gphysicsRuntime.h>
#include <gemc/gsd/gsd.h>
#include <gemc/gui/physicsView.h>
#include <gemc/gboard/gboard.h>
#include <gemc/gboard/gui_session.h>

#include <QApplication>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QTabWidget>
#include <QTreeWidget>
#include <QElapsedTimer>

#include "G4Electron.hh"
#include "G4Event.hh"
#include "G4HCofThisEvent.hh"
#include "G4MTRunManager.hh"
#include "G4OpticalPhoton.hh"
#include "G4ParticleGun.hh"
#include "G4ProcessManager.hh"
#include "G4ProductionCuts.hh"
#include "G4RegionStore.hh"
#include "G4RunManagerFactory.hh"
#include "G4SystemOfUnits.hh"
#include "G4Track.hh"
#include "G4UImanager.hh"
#include "G4UserEventAction.hh"
#include "G4UserRunAction.hh"
#include "G4UserTrackingAction.hh"
#include "G4VProcess.hh"
#include "G4VUserActionInitialization.hh"
#include "G4VUserPrimaryGeneratorAction.hh"

#include <atomic>
#include <cmath>
#include <iostream>
#include <memory>
#include <mutex>
#include <set>
#include <stdexcept>

namespace {

struct Counts {
	std::atomic<int> photons{0}, cherenkov{0}, hits{0}, events{0}, badInventory{0}, workers{0};
	bool optical = true;
	bool activated = true;
	double cut = 0.1;
	void reset() {
		photons = 0; cherenkov = 0; hits = 0; events = 0; badInventory = 0; workers = 0;
	}
};

class Primary : public G4VUserPrimaryGeneratorAction {
public:
	Primary() : gun(1) {
		gun.SetParticleDefinition(G4Electron::Definition());
		gun.SetParticleEnergy(1 * GeV);
		gun.SetParticlePosition({0, 0, -499 * mm});
		gun.SetParticleMomentumDirection({0, 0, 1});
	}
	void GeneratePrimaries(G4Event* event) override { gun.GeneratePrimaryVertex(event); }
private:
	G4ParticleGun gun;
};

class Tracking : public G4UserTrackingAction {
public:
	explicit Tracking(Counts& input) : counts(input) {}
	void PreUserTrackingAction(const G4Track* track) override {
		if (track->GetDefinition() != G4OpticalPhoton::Definition()) { return; }
		++counts.photons;
		if (track->GetCreatorProcess() && track->GetCreatorProcess()->GetProcessName() == "Cerenkov") {
			++counts.cherenkov;
		}
	}
private:
	Counts& counts;
};

class Events : public G4UserEventAction {
public:
	explicit Events(Counts& input) : counts(input) {}
	void EndOfEventAction(const G4Event* event) override {
		++counts.events;
		auto* collections = const_cast<G4HCofThisEvent*>(event->GetHCofThisEvent());
		if (!collections) { return; }
		for (int i = 0; i < collections->GetNumberOfCollections(); ++i) {
			auto* hits = static_cast<GHitsCollection*>(collections->GetHC(i));
			if (hits && hits->GetSDname() == "gPhotonDetector") { counts.hits += hits->GetSize(); }
		}
	}
private:
	Counts& counts;
};

class Runs : public G4UserRunAction {
public:
	explicit Runs(Counts& input) : counts(input) {}
	void BeginOfRunAction(const G4Run*) override {
		++counts.workers;
		auto* manager = G4Electron::Definition()->GetProcessManager();
		int cerenkov = 0, transportation = 0;
		bool active = false;
		static thread_local const G4VProcess* originalIonisation = nullptr;
		for (int i = 0; i < manager->GetProcessListLength(); ++i) {
			const auto* process = (*manager->GetProcessList())[i];
			if (process->GetProcessName() == "Cerenkov") {
				++cerenkov;
				active = manager->GetProcessActivation(i);
			}
			if (process->GetProcessType() == fTransportation) { ++transportation; }
			if (process->GetProcessType() == fTransportation &&
			    (manager->GetProcessOrdering(const_cast<G4VProcess*>(process), idxAlongStep) != 0 ||
			     manager->GetProcessOrdering(const_cast<G4VProcess*>(process), idxPostStep) != 0)) {
				++counts.badInventory;
			}
			if (process->GetProcessName() == "eIoni") {
				if (!originalIonisation) { originalIonisation = process; }
				if (originalIonisation != process) { ++counts.badInventory; }
			}
		}
		const auto* cuts = G4RegionStore::GetInstance()->GetRegion("DefaultRegionForTheWorld")
		                   ->GetProductionCuts();
		if (cerenkov != int(counts.optical) || transportation != 1 ||
		    (counts.optical && active != counts.activated) ||
		    std::abs(cuts->GetProductionCut("gamma") / mm - counts.cut) > 1e-10) {
			++counts.badInventory;
		}
		for (const auto& state : gphysics::RuntimePhysicsList::current()->processes()) {
			if (state.name == "Cerenkov" && state.category != "Optical") { ++counts.badInventory; }
			if ((state.name == "msc" || state.name == "hadElastic") &&
			    (state.mixed || state.active != counts.activated)) { ++counts.badInventory; }
		}
		for (const auto& name : {"gamma", "e-", "e+", "proton"}) {
			if (std::abs(cuts->GetProductionCut(name) / mm - counts.cut) > 1e-10) { ++counts.badInventory; }
		}
	}
private:
	Counts& counts;
};

class Actions : public G4VUserActionInitialization {
public:
	explicit Actions(Counts& input) : counts(input) {}
	void Build() const override {
		SetUserAction(new Primary);
		SetUserAction(new Tracking(counts));
		SetUserAction(new Events(counts));
		SetUserAction(new Runs(counts));
	}
private:
	Counts& counts;
};

void require(bool condition, const std::string& message) {
	if (!condition) { throw std::runtime_error(message); }
}

} // namespace

int main(int argc, char* argv[]) {
	auto schema = gemc::defineOptions();
	schema.defineOption(GVariable("physics_test_mode", "serial", "run-manager type for physics tests"), "");
	auto options = std::make_shared<GOptions>(argc, argv, schema);
	const auto mode = options->getRequiredScalarString("physics_test_mode");
	std::unique_ptr<QApplication> application;
	if (mode == "gui") { application = std::make_unique<QApplication>(argc, argv); }
	const auto type = mode == "serial" || mode == "gui" ? G4RunManagerType::SerialOnly :
	                  mode == "mt" ? G4RunManagerType::MTOnly : G4RunManagerType::TaskingOnly;
	std::unique_ptr<GBoard> board;
	std::unique_ptr<GUI_Session> session;
	if (application) {
		board = std::make_unique<GBoard>(options);
		board->resize(900, 400);
		board->show();
		session = std::make_unique<GUI_Session>(options, board.get());
		for (int i = 0; i < 1500; ++i) { board->appendLog(QString("console history %1").arg(i)); }
		application->processEvents();
		auto* display = board->findChild<QTextEdit*>();
		int removed = 0;
		const auto connection = QObject::connect(display->document(), &QTextDocument::contentsChange,
		                                        [&](int, int count, int) { removed += count; });
		board->appendLog("<b>new console output</b>");
		application->processEvents();
		require(removed == 0 && display->toPlainText().endsWith("new console output"),
		        "Appending console output rebuilt existing history");
		QObject::disconnect(connection);
		auto* filter = board->findChild<QLineEdit*>("searchLineEdit");
		filter->setText("NEW CONSOLE");
		require(display->toPlainText() == "new console output", "Console filter lost history");
		board->appendLog("hidden while filtered");
		application->processEvents();
		require(display->toPlainText() == "new console output", "Filtered output became visible");
		filter->clear();
		require(display->toPlainText().contains("console history 1499") &&
		        display->toPlainText().endsWith("hidden while filtered"), "Console history was discarded");
		board->appendLog("queued before clear");
		QMetaObject::invokeMethod(board.get(), "clearLog", Qt::DirectConnection);
		application->processEvents();
		require(!display->toPlainText().contains("queued before clear"), "Cleared console output reappeared");
	}
	auto* physics = new gphysics::RuntimePhysicsList("FTFP_BERT+G4OpticalPhysics");
	std::string error;
	// Applications may configure physics before constructing a run manager or its default region.
	require(physics->setProductionCut(0.1, error), error);
	auto manager = std::unique_ptr<G4RunManager>(G4RunManagerFactory::CreateRunManager(type));
	if (auto* mt = dynamic_cast<G4MTRunManager*>(manager.get())) { mt->SetNumberOfThreads(2); }
	manager->SetUserInitialization(physics);
	manager->SetUserInitialization(new GDetectorConstruction(options));
	Counts counts;
	manager->SetUserInitialization(new Actions(counts));
	manager->Initialize();
	std::unique_ptr<PhysicsView> view;
	if (application) {
		view = std::make_unique<PhysicsView>(options);
		view->resize(900, 850);
		view->show();
		application->processEvents();
	}

	for (int phase = 0; phase < 5; ++phase) {
		std::cout << "PHYSICS_START " << phase << std::endl;
		QElapsedTimer applyTimer;
		applyTimer.start();
		counts.reset();
		counts.optical = phase != 1;
		counts.activated = phase != 3;
		counts.cut = phase == 1 ? 2.0 : 0.1;
		const auto selection = counts.optical ? "FTFP_BERT+G4OpticalPhysics" : "FTFP_BERT";
		if (view) {
			view->findChild<QDoubleSpinBox*>("gemcProductionCut")->setValue(counts.cut);
			auto* tabs = view->findChild<QTabWidget*>("gemcPhysicsConstructors");
			if (phase == 1) { tabs->tabCloseRequested(0); }
			if (phase == 2) { tabs->tabBarClicked(tabs->count() - 1); }
			if (phase >= 3) {
				auto* tree = view->findChild<QTreeWidget*>("gemcPhysicsProcesses");
				for (const auto& name : {"msc", "hadElastic"}) {
					const auto items = tree->findItems(name, Qt::MatchExactly | Qt::MatchRecursive);
					require(items.size() == 1, "Missing or duplicate process checkbox");
					items.first()->setCheckState(0, counts.activated ? Qt::Checked : Qt::Unchecked);
				}
				const auto optical = tree->findItems("Optical", Qt::MatchExactly);
				require(optical.size() == 1, "Missing optical-process category checkbox");
				optical.first()->setCheckState(0, counts.activated ? Qt::Checked : Qt::Unchecked);
			}
			require(view->applyPending(), "GUI physics changes failed");
		} else {
			require(physics->select(selection, error), error);
			require(physics->setProductionCut(counts.cut, error), error);
		}
		if (phase >= 3 && !view) {
			for (const auto& name : {"Cerenkov", "msc", "hadElastic"}) {
				require(physics->setProcessEnabled(name, counts.activated, error), error);
			}
		}
		if (application) { application->processEvents(); }
		std::cout << "PHYSICS_CONFIGURED " << phase << " apply_ms=" << applyTimer.elapsed() << std::endl;
		manager->BeamOn(100);
		if (view) { view->refreshProcesses(); }
		std::cout << "PHYSICS_PHASE " << phase << " mode=" << mode
		          << " photons=" << counts.photons << " cerenkov=" << counts.cherenkov
		          << " hits=" << counts.hits << " workers=" << counts.workers
		          << " inventory_errors=" << counts.badInventory << '\n';
		const bool light = counts.optical && counts.activated;
		require(counts.events == 100, "Wrong event count");
		require(counts.badInventory == 0, "Worker retained stale or duplicate processes/cuts");
		require(counts.workers == (mode == "serial" || mode == "gui" ? 1 : 2),
		        "Missing worker process inventory");
		require(light ? counts.photons > 0 && counts.cherenkov > 0 && counts.hits > 0 :
		                counts.photons == 0 && counts.hits == 0, "Unexpected optical-photon production");
	}
	counts.reset();
	if (view) {
		view->findChild<QComboBox*>("gemcPhysicsReference")->setCurrentText("QGSP_BERT");
		require(view->applyPending(), "GUI reference-list change failed");
	} else {
		require(physics->select("QGSP_BERT+G4OpticalPhysics", error), error);
	}
	manager->BeamOn(10);
	require(counts.events == 10 && counts.photons > 0 && counts.badInventory == 0,
	        "Compatible reference-list change did not propagate to workers");
	std::cout << "PHYSICS_REFERENCE_SWITCH " << mode << " photons=" << counts.photons << '\n';
	require(G4UImanager::GetUIpointer()->ApplyCommand("/run/setCut 0.25 mm") == 0 &&
	        std::abs(physics->GetDefaultCutValue() / mm - 0.25) < 1e-10,
	        "Native Geant4 cut command no longer addresses the active physics list");
	require(physics->setProductionCut(0.1, error), error);
	const auto selected = physics->selection();
	require(!physics->select("NO_SUCH_PHYSICS_LIST", error) && physics->selection() == selected,
	        "Invalid physics selection changed the active list");
	require(!physics->select("FTFP_BERT+G4OpticalPhysics+G4OpticalPhysics", error),
	        "Accepted duplicate physics constructors");
	require(!physics->select("FTFP_BERT+G4EmStandardPhysics_option4", error),
	        "Silently ignored a conflicting physics constructor");
	require(!physics->setProductionCut(-1, error), "Accepted a negative production cut");
	require(!physics->setProcessEnabled("Transportation", false, error), "Disabled transportation");
	require(!physics->select("FTFP_BERT_EMZ+G4OpticalPhysics", error) &&
	        physics->selection() == selected, "Changed electromagnetic constructor after initialization");
	counts.reset();
	manager->BeamOn(10);
	require(counts.events == 10 && counts.photons > 0 && counts.badInventory == 0,
	        "Rejected selection damaged the active physics configuration");
	if (view && std::getenv("GEMC_PHYSICS_SNAPSHOT")) {
		view->grab().save(QString::fromLocal8Bit(std::getenv("GEMC_PHYSICS_SNAPSHOT")));
	}
	view.reset();
	manager.reset(); // Exercise teardown of both active and retired physics constructors.
	if (application) {
		application->processEvents();
		require(!board->findChild<QTextEdit*>()->toPlainText().contains("ProcMan114"),
		        "Optical switching repeated process-ordering warnings in the GUI console");
	}
	session.reset();
	std::cout << "PHYSICS_SWITCHING_PASSED " << mode << '\n';
	return 0;
}
