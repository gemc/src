// gemc
#include "gdetectorConstruction.h"

// glibrary
#include "gworld.h"


// geant4
//#include "G4SDManager.hh"


GDetectorConstruction::GDetectorConstruction(GOptions* opt, map<string, GDynamicDigitization*> *gDDGlobal) :
G4VUserDetectorConstruction(),                            // geant4 derived
GStateMessage(opt, "GDetectorConstruction", "gsystemv"),  // GStateMessage derived
gopt(opt),
gDynamicDigitizationMapGlobalInstance(gDDGlobal)
{
	// should this go here or in Construct()


	// building gemc world (systems containings gvolumes)
	gworld = new GWorld(gopt);

	// builiding geant4 world (solid, logical, physical volumes)
	g4world = new G4World(gworld, gopt);


	

	
}

// delete the two pointers created by GDetectorConstruction
GDetectorConstruction::~GDetectorConstruction() {
	delete gworld;
	delete g4world;
}

G4VPhysicalVolume* GDetectorConstruction::Construct()
{
	logDetail("Constructing gemc world");

	// loading gvolumes, material, system parameters

//
//	gworld = new GWorld(gopt);
//
//	return g4setup->getPhysical(WORLDNAME);


	return nullptr;
}

void GDetectorConstruction::ConstructSDandField()
{
	// no need to do anything if we're in the main thread
	// PRAGMA TODO:
	// If we return here if it's master, then the hit will not be
	// processed in GSensitiveDetector::ProcessHits
	if (G4Threading::IsMasterThread() ) return;

	// int verbosity = gopt->getInt("gsensitivityv");

//	flowMessage("Inside SDandField");
//
//	// used to check if a SD if it already exists
//	map<string, GSensitiveDetector*> allSensitiveDetectors;


//	// building the sensitive detectors
//	// this is thread local
//	for(auto &s : gsetup->getSetup()) {
//		for(auto &gv : s.second->getSytems()) {
//			string sensitivity = gv.second->getSensitivity();
//
//			// making sure the logical volume exists
//			if(g4setup->getLogical(gv.first) == nullptr) {
//				G4cerr << FATALERRORL << "  Error: " << gv.first << " logical volume not build? This should never happen." << G4endl;
//				exit(99);
//			} else if(sensitivity != NOTAPPLICABLE) {
//				// checking that we do not already have a GSensitiveDetector
//				if(allSensitiveDetectors.find(sensitivity) == allSensitiveDetectors.end()) {
//
//					if(verbosity == GVERBOSITY_ALL) {
//						G4cout  << "Sensitive detector " << sensitivity << " doesn't exist for " << gv.first << ". Creating it." << G4endl;
//					}
//
//					allSensitiveDetectors[sensitivity] = new GSensitiveDetector(sensitivity, gopt, gDigitizationGlobal);
//					// PRAGMA TODO: according to the documentation the AddNewDetector is done by SetSensitiveDetector
//					// however GSensitiveDetector::Initialize will not work if this is not done here
//					auto sdManager = G4SDManager::GetSDMpointer();
//					sdManager->AddNewDetector(allSensitiveDetectors[sensitivity]);
//
//				} else {
//					if(verbosity == GVERBOSITY_ALL) {
//						G4cout <<  "Sensitive detector " << sensitivity << " exists for " << gv.first << G4endl;
//					}
//				}
//				allSensitiveDetectors[sensitivity]->registerGVolumeTouchable(gv.first, new GTouchable(sensitivity, gv.second->getTouchableID()));
//				SetSensitiveDetector(gv.first, allSensitiveDetectors[sensitivity]);
//			}
//		}
//	}


}







