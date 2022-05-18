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
}

// delete the two pointers created by GDetectorConstruction
GDetectorConstruction::~GDetectorConstruction() {
		delete gworld;
		delete g4world;
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
	logSummary("GDetectorConstruction::ConstructSDandField");
	
	// GSensitiveDetector map
	map<string, GSensitiveDetector*> sensitiveDetectorsMap;

	// building the sensitive detectors
	// this is thread local
	for(auto [systemName, gsystem] : *gworld->getSystemsMap()) {
		for(auto [volumeName, gvolume] : *gsystem->getGVolumesMap()) {
			
			string digitizationName = gvolume->getDigitization();
			
			// skip root or no digitization
			if (volumeName == ROOTWORLDGVOLUMENAME || digitizationName == UNINITIALIZEDSTRINGQUANTITY) {
				continue;
			}
			
			// making sure the geant4 logical volume exists
			string g4name = gvolume->getG4Name();
			if(g4world->getG4Volume(g4name) == nullptr) {
				G4cerr << FATALERRORL << "  Error: <" << g4name << "> logical volume not build? This should never happen." << G4endl;
				exit(99);
			} else {
				
				// checking that we do not already have a GSensitiveDetector
				if(sensitiveDetectorsMap.find(digitizationName) == sensitiveDetectorsMap.end()) {
					
					logSummary("Sensitive detector <" + digitizationName + "> doesn't exist for <" + g4name + ">. Creating it.");
					sensitiveDetectorsMap[digitizationName] = new GSensitiveDetector(digitizationName, gopt, gDynamicDigitizationMapGlobalInstance);

					auto sdManager = G4SDManager::GetSDMpointer();
					sdManager->SetVerboseLevel(10);
					sdManager->AddNewDetector(sensitiveDetectorsMap[digitizationName]);

				} else {
					logSummary("Sensitive detector <" + digitizationName + "> exist for <" + volumeName + ">");
				}

				sensitiveDetectorsMap[digitizationName]->registerGVolumeTouchable(g4name, new GTouchable(digitizationName,
																																	  gvolume->getGIdentity(),
																																	  gvolume->getDetectorDimensions()));

				SetSensitiveDetector(g4name, sensitiveDetectorsMap[digitizationName]);

			}
		}
	}
}





