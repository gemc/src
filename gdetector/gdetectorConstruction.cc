// gemc
#include "gdetectorConstruction.h"

// glibrary
#include "gworld.h"
#include "g4systemConventions.h"

// geant4
#include "G4SDManager.hh"

GDetectorConstruction::GDetectorConstruction(GOptions* opt, map<string, GDynamicDigitization*> *gDDGlobal) :
G4VUserDetectorConstruction(),                                   // geant4 derived
GStateMessage(opt, "GDetectorConstruction", G4SYSTEMVERBOSITY),  // GStateMessage derived
gopt(opt),
gDynamicDigitizationMapGlobalInstance(gDDGlobal)
{
	//	// building gemc world (systems containings gvolumes)
	//	gworld = new GWorld(gopt);
	//
	//	// builiding geant4 world (solid, logical, physical volumes)
	//	g4world = new G4World(gworld, gopt);
	
}

// delete the two pointers created by GDetectorConstruction
GDetectorConstruction::~GDetectorConstruction() {
	//	delete gworld;
	//	delete g4world;
}

G4VPhysicalVolume* GDetectorConstruction::Construct()
{
	logSummary("GDetectorConstruction::Construct");
	
	// building gemc world (systems containings gvolumes)
	gworld = new GWorld(gopt);

	// builiding geant4 world (solid, logical, physical volumes)
	g4world = new G4World(gworld, gopt);



	return g4world->getG4Volume(ROOTWORLDGVOLUMENAME)->getPhysical();
}

// thread local beware
void GDetectorConstruction::ConstructSDandField()
{
	// no need to do anything if we're in the main thread
	// PRAGMA TODO:
	// If we return here if it's master, then the hit will not be
	// processed in GSensitiveDetector::ProcessHits
	if (G4Threading::IsMasterThread() ) return;
	
	
	logSummary("GDetectorConstruction::ConstructSDandField");
	
	// GSensitiveDetector map
	map<string, GSensitiveDetector*> sensitiveDetectorsMap;
	
	
	// building the sensitive detectors
	// this is thread local
	for(auto [systemName, gsystem] : *gworld->getSystemsMap()) {
		for(auto [volumeName, gvolume] : *gsystem->getGVolumesMap()) {
			
			string digitizationName = gvolume->getDigitization();
			
			// skip root or no digitization
			if (volumeName == ROOTWORLDGVOLUMENAME || digitizationName == GSYSTEMNOTAPPLICABLEENTRY) {
				continue;
			}
			
			// making sure the geant4 logical volume exists
			if(g4world->getG4Volume(volumeName) == nullptr) {
				G4cerr << FATALERRORL << "  Error: <" << volumeName << "> logical volume not build? This should never happen." << G4endl;
				exit(99);
			} else {
				
				// checking that we do not already have a GSensitiveDetector
				if(sensitiveDetectorsMap.find(digitizationName) == sensitiveDetectorsMap.end()) {
					
					logSummary("Sensitive detector <" + digitizationName + "> doesn't exist for <" + volumeName + ">. Creating it.");
					
					G4cout << "Sensitive detector <" << digitizationName << "> doesn't exist for <" << volumeName << ">. Creating it." << G4endl;

					cout << "Sensitive detector <" << digitizationName << "> doesn't exist for <" << volumeName << ">. Creating it." << endl;

					sensitiveDetectorsMap[digitizationName] = new GSensitiveDetector(digitizationName, gopt, gDynamicDigitizationMapGlobalInstance);

					// PRAGMA TODO: according to the documentation the AddNewDetector is done by SetSensitiveDetector
					// however GSensitiveDetector::Initialize will not work if this is not done here
					//					auto sdManager = G4SDManager::GetSDMpointer();
					//					sdManager->AddNewDetector(sensitiveDetectorsMap[digitizationName]);
					
				} else {
					logSummary("Sensitive detector <" + digitizationName + "> exist for <" + volumeName + ">");
				}
				auto sdManager = G4SDManager::GetSDMpointer();
				sdManager->SetVerboseLevel(10);

				sensitiveDetectorsMap[digitizationName]->registerGVolumeTouchable(volumeName, new GTouchable(digitizationName, gvolume->getGIdentity()));

				sdManager->AddNewDetector(sensitiveDetectorsMap[digitizationName]);

				//				if(volumeName == "ch__ch") {
				//
				//				cout << "QQQQ " << sdManager->FindSensitiveDetector("aa") << " " << sdManager->FindSensitiveDetector("ch") << endl;
				//				}


				cout << " ASD " << volumeName << " " << g4world->getG4Volume(volumeName)->getLogical()->GetName() << " "  << g4world->getG4Volume(volumeName)->getLogical()  << endl;
				

				// SetSensitiveDetector is called by G4VUserDetectorConstruction call
				//SetSensitiveDetector(g4world->getG4Volume(volumeName)->getLogical(), thisSensitive);
				SetSensitiveDetector(volumeName, sensitiveDetectorsMap[digitizationName]);
				//g4world->getG4Volume(volumeName)->getLogical()->SetSensitiveDetector(sensitiveDetectorsMap[digitizationName]);
				
			}
		}
	}
}





