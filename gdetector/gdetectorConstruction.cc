// gemc
#include "gdetectorConstruction.h"
#include "gworld.h"
#include "g4systemConventions.h"

//#include "goptions.h"
//#include "gfactory.h"
#include "gtouchableConventions.h"
#include "ginternalDigitization.h"
//#include "gemcUtilities.h"
//#include "gemcConventions.h"

// geant4
#include "G4SDManager.hh"

G4ThreadLocal GMagneto* GDetectorConstruction::gmagneto = nullptr;

GDetectorConstruction::GDetectorConstruction(GOptions *gopts,
                                             map<string, GDynamicDigitization *> *gDDGlobal) :
        G4VUserDetectorConstruction(),                                                 // Geant4 derived
        GStateMessage(gopts, "GDetectorConstruction", "g4system"),  // GStateMessage derived
        gopt(gopts),
        gDynamicDigitizationMapGlobalInstance(gDDGlobal) {}

// delete the two pointers created by GDetectorConstruction
GDetectorConstruction::~GDetectorConstruction() {
    delete gworld;
    delete g4world;
    delete gmagneto;
}

G4VPhysicalVolume *GDetectorConstruction::Construct() {
    logSummary("GDetectorConstruction::Construct");

    // building gemc world (systems containings gvolumes)
    gworld = new GWorld(gopt);

    // builiding geant4 world (solid, logical, physical volumes)
    g4world = new G4World(gworld, gopt);

    return g4world->getG4Volume(ROOTWORLDGVOLUMENAME)->getPhysical();
}

// thread local beware
void GDetectorConstruction::ConstructSDandField() {
    logSummary("GDetectorConstruction::ConstructSDandField ");

    bool touchableVerbosity = false;
    if (gopt->getVerbosityFor("gsensitivity") >= GVERBOSITY_DETAILS) touchableVerbosity = true;

    // GSensitiveDetector map
    map < string, GSensitiveDetector * > sensitiveDetectorsMap;

    // building the sensitive detectors
    // this is thread local
    for (auto [systemName, gsystem]: *gworld->getSystemsMap()) {
        for (auto [volumeName, gvolume]: *gsystem->getGVolumesMap()) {

            string digitizationName = gvolume->getDigitization();
            string g4name = gvolume->getG4Name();
            // making sure the geant4 logical volume exists
            if (g4world->getG4Volume(g4name) == nullptr) {
                G4cerr << FATALERRORL << "  Error: <" << g4name
                       << "> logical volume not build? This should never happen." << G4endl;
                exit(99);  // not assigned a value
            }
            // skip no digitization
            if (digitizationName != UNINITIALIZEDSTRINGQUANTITY) {

                // checking that we do not already have a GSensitiveDetector
                if (sensitiveDetectorsMap.find(digitizationName) == sensitiveDetectorsMap.end()) {

                    logSummary("Sensitive detector <" + digitizationName + "> doesn't exist for <" + g4name
                               + ">. Creating it.");
                    sensitiveDetectorsMap[digitizationName] = new GSensitiveDetector(digitizationName,
                                                                                     gopt,
                                                                                     gDynamicDigitizationMapGlobalInstance);

                    auto sdManager = G4SDManager::GetSDMpointer();
                    sdManager->SetVerboseLevel(10);
                    sdManager->AddNewDetector(sensitiveDetectorsMap[digitizationName]);

                } else {
                    logDetail("Sensitive detector <" + digitizationName + "> exist for <" + volumeName + ">");
                }

                // TODO: the last option should come from options, by default is false
                sensitiveDetectorsMap[digitizationName]->registerGVolumeTouchable(g4name,
                                                                                  new GTouchable(digitizationName,
                                                                                                 gvolume->getGIdentity(),
                                                                                                 gvolume->getDetectorDimensions(),
                                                                                                 touchableVerbosity));

                SetSensitiveDetector(g4name, sensitiveDetectorsMap[digitizationName]);
            }

            string field_name = gvolume->getEMField();
            if (field_name != UNINITIALIZEDSTRINGQUANTITY) {
                if (gmagneto == nullptr) gmagneto = new GMagneto(gopt);
                logDetail("Volume  <" + volumeName + "> has field: <" + field_name + ">. Looking into field map definitions.");

                // assigning field manager to the volume
                logDetail("Setting field manager for volume <" + g4name + "> with field <" + field_name + ">");
                g4world->setFieldManagerForVolume(g4name, gmagneto->getFieldMgr(field_name), true);

            }
        }
    }

	loadDigitizationPlugins();
}

void  GDetectorConstruction::loadDigitizationPlugins() {

	vector<string> sdetectors = gworld->getSensitiveDetectorsList();

	int verbosity = gopt->getVerbosityFor("gsensitivity");

	for (auto &sdname: sdetectors) {

		if (sdname == FLUXNAME) {
			(*gDynamicDigitizationMapGlobalInstance)[sdname] = new GFluxDigitization();
			(*gDynamicDigitizationMapGlobalInstance)[sdname]->defineReadoutSpecs();
		} else if (sdname == COUNTERNAME) {
			(*gDynamicDigitizationMapGlobalInstance)[sdname] = new GParticleCounterDigitization();
			(*gDynamicDigitizationMapGlobalInstance)[sdname]->defineReadoutSpecs();
		} else if (sdname == DOSIMETERNAME) {
			(*gDynamicDigitizationMapGlobalInstance)[sdname] = new GDosimeterDigitization();
			(*gDynamicDigitizationMapGlobalInstance)[sdname]->defineReadoutSpecs();

		} else {

			if (verbosity >= GVERBOSITY_SUMMARY) {
				cout  << "Loading plugins from file " << sdname << endl;
			}

			GManager sdPluginManager(sdname + " GSensitiveDetector", verbosity);

			if (gDynamicDigitizationMapGlobalInstance->find(sdname) == gDynamicDigitizationMapGlobalInstance->end()) {
				(*gDynamicDigitizationMapGlobalInstance)[sdname] = sdPluginManager.LoadAndRegisterObjectFromLibrary<GDynamicDigitization>(sdname);
				(*gDynamicDigitizationMapGlobalInstance)[sdname]->defineReadoutSpecs();

			}

			// done with sdPluginManager
			//sdPluginManager.clearDLMap();
		}
	}
}

