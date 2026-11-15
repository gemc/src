#include "gemc_options.h"
#include <gemc/actions/gaction.h>
#include <gemc/gdetector/gdetectorConstruction.h>
#include <gemc/gphysics/gphysics.h>
#include "G4RunManagerFactory.hh"
#include "G4SDManager.hh"
#include "G4VSensitiveDetector.hh"

#include <memory>

namespace {
class InactiveDetector : public G4VSensitiveDetector {
public:
	InactiveDetector() : G4VSensitiveDetector("unused") { collectionName.insert("unused"); }
	G4bool ProcessHits(G4Step*, G4TouchableHistory*) override { return false; }
};

class SparseDetector : public GDetectorConstruction {
public:
	using GDetectorConstruction::GDetectorConstruction;
	void ConstructSDandField() override {
		// Reserve collection ID 0 in every execution context, then leave its slot empty.
		auto* inactive = new InactiveDetector;
		G4SDManager::GetSDMpointer()->AddNewDetector(inactive);
		inactive->Activate(false);
		GDetectorConstruction::ConstructSDandField();
	}
};
}

int main(int argc, char* argv[]) {
	auto options = std::make_shared<GOptions>(argc, argv, gemc::defineOptions());
	auto manager = std::unique_ptr<G4RunManager>(
		G4RunManagerFactory::CreateRunManager(G4RunManagerType::MTOnly));
	manager->SetNumberOfThreads(1);
	auto* detector = new SparseDetector(options);
	manager->SetUserInitialization(detector);
	auto* physics = new GPhysics(options);
	manager->SetUserInitialization(physics->getPhysList());
	manager->SetUserInitialization(new GAction(options, detector->get_digitization_routines_map()));
	manager->Initialize();
	manager->BeamOn(1);
	return 0;
}
