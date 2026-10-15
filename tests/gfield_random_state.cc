#include <gemc/gfields/gfield_options.h>
#include <gemc/gfields/gmagneto.h>

#include "CLHEP/Random/RanluxEngine.h"
#include "FTFP_BERT.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4MTRunManager.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4RunManagerFactory.hh"
#include "G4SystemOfUnits.hh"
#include "G4VUserActionInitialization.hh"
#include "G4VUserDetectorConstruction.hh"
#include "G4VUserPrimaryGeneratorAction.hh"

#include <cstdlib>
#include <memory>
#include <sstream>
#include <stdexcept>

namespace {

class Detector : public G4VUserDetectorConstruction {
public:
	explicit Detector(std::shared_ptr<GOptions> input) : options(std::move(input)) {}

	G4VPhysicalVolume* Construct() override {
		auto* material = G4NistManager::Instance()->FindOrBuildMaterial("G4_Galactic");
		logical = new G4LogicalVolume(new G4Box("world", CLHEP::m, CLHEP::m, CLHEP::m), material, "world");
		return new G4PVPlacement(nullptr, {}, logical, "world", nullptr, false, 0);
	}

	void ConstructSDandField() override {
		// Match GEMC's per-thread registry lifetime; Geant4 cleans up the registered field managers.
		static G4ThreadLocal GMagneto* magneto = nullptr;
		if (!magneto) { magneto = new GMagneto(options); }
		logical->SetFieldManager(magneto->getFieldMgr("dipole").get(), true);
	}

private:
	std::shared_ptr<GOptions> options;
	G4LogicalVolume* logical = nullptr;
};

class Primary : public G4VUserPrimaryGeneratorAction {
public:
	void GeneratePrimaries(G4Event*) override {}
};

class Actions : public G4VUserActionInitialization {
public:
	void Build() const override { SetUserAction(new Primary); }
};

} // namespace

int main(int argc, char* argv[]) {
	auto options = std::make_shared<GOptions>(argc, argv, gfields::defineOptions());
	CLHEP::HepRandom::setTheEngine(new CLHEP::RanluxEngine(12345));
	auto manager = std::unique_ptr<G4RunManager>(
		G4RunManagerFactory::CreateRunManager(G4RunManagerType::Tasking));
	dynamic_cast<G4MTRunManager*>(manager.get())->SetNumberOfThreads(2);
	manager->SetUserInitialization(new Detector(options));
	manager->SetUserInitialization(new FTFP_BERT);
	manager->SetUserInitialization(new Actions);

	// Geant4 builds worker geometry between cloning and saving the engine. Duplicate plugin geometry
	// state previously overwrote the engine's vtable; a plain std::thread RNG probe misses this failure.
	manager->Initialize();
	std::stringstream state;
	CLHEP::HepRandom::saveFullState(state);
	if (!state || state.str().empty()) { throw std::runtime_error("Could not save random-engine state"); }
	return EXIT_SUCCESS;
}
