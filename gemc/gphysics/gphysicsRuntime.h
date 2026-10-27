#pragma once

#include "G4VModularPhysicsList.hh"
#include "G4Cache.hh"

#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>

class GPhysicsMessenger;
class G4VProcess;

namespace gphysics {

struct ProcessState {
	std::string name;
	std::string category;
	bool active = true;
	bool mixed = false;
	bool editable = true;
	std::vector<std::string> particles;
};

/** Keeps the run-manager physics pointer stable while rebuilding compatible physics between runs.
 * The EM constructor must remain unchanged after initialization: Geant4 caches its process pointers.
 */
class RuntimePhysicsList : public G4VModularPhysicsList {
public:
	explicit RuntimePhysicsList(const std::string& selection);
	~RuntimePhysicsList() override;
	void ConstructParticle() override;
	void ConstructProcess() override;
	void SetCuts() override;
	void InitializeWorker() override;
	void TerminateWorker() override;

	bool select(const std::string& selection, std::string& error);
	bool setProductionCut(double millimeters, std::string& error);
	bool setProcessEnabled(const std::string& name, bool enabled, std::string& error);
	[[nodiscard]] const std::string& selection() const { return selected; }
	[[nodiscard]] std::vector<ProcessState> processes() const;
	[[nodiscard]] static std::vector<std::string> referenceLists();
	[[nodiscard]] static std::vector<std::string> constructors();
	[[nodiscard]] static RuntimePhysicsList* current();

private:
	friend class ::GPhysicsMessenger;
	void clearProcesses();
	void rebuildWorker();
	void applyActivation();
	void broadcast(const std::string& command);
	struct ThreadState {
		bool initialized = false;
		std::set<G4VProcess*> retained;
		std::vector<G4VPhysicsConstructor*> retainedConstructors;
	};

	// Geant4 list destructors delete the global particle table. Keep factory lists until shutdown;
	// non-EM processes are removed at each switch, and factory lists retain constructor ownership.
	std::vector<std::unique_ptr<G4VModularPhysicsList>> lists;
	G4VModularPhysicsList* activeList = nullptr;
	std::vector<G4VPhysicsConstructor*> activeConstructors;
	G4Cache<ThreadState> threadState;
	std::unique_ptr<GPhysicsMessenger> messenger;
	std::map<std::string, bool> activation;
	std::optional<double> productionCut;
	std::string selected;
	std::string electromagneticIdentity;
};

} // namespace gphysics
