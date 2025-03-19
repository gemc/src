// gemc
#include "gdetectorConstruction.h"
#include "g4systemConventions.h"
#include "gtouchableConventions.h"
#include "ginternalDigitization.h"

// geant4
#include "G4SDManager.hh"
#include "G4GeometryManager.hh"
#include "G4SolidStore.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4ReflectionFactory.hh"
#include "G4RunManager.hh"

#include <iostream>
#include <cstdlib>

using std::cerr;
using std::cout;
using std::endl;

G4ThreadLocal GMagneto *GDetectorConstruction::gmagneto = nullptr;

GDetectorConstruction::GDetectorConstruction(GOptions *gopts,
											 map<string, GDynamicDigitization *> *gDDGlobal)
		: G4VUserDetectorConstruction(),                                // Geant4 base class.
		  GStateMessage(gopts, "GDetectorConstruction", "g4system"),     // GEMC state message.
		  gopt(gopts),
		  gDynamicDigitizationMapGlobalInstance(gDDGlobal)
{
	// Ensure the local GSystem vector is empty at start.
	gsystems.clear();
}

GDetectorConstruction::~GDetectorConstruction() {
	// Clean up the GEMC and Geant4 world objects.
	delete gworld;
	delete g4world;
	delete gmagneto;
}

G4VPhysicalVolume *GDetectorConstruction::Construct() {
	logSummary("GDetectorConstruction::Construct");

	// Clean any old geometry.
	G4GeometryManager::GetInstance()->OpenGeometry();
	G4PhysicalVolumeStore::GetInstance()->Clean();
	G4LogicalVolumeStore::GetInstance()->Clean();
	G4SolidStore::GetInstance()->Clean();
	G4ReflectionFactory::Instance()->Clean();

	// Delete old geometry objects if they exist.
	if (gworld) { delete gworld; gworld = nullptr; }
	if (g4world) { delete g4world; g4world = nullptr; }

	// Build GEMC world: if no systems are provided, create from options; otherwise, use existing systems.
	if (gsystems.empty()) {
		gworld = new GWorld(gopt);
	} else {
		gworld = new GWorld(gopt, gsystems);
	}

	// Build Geant4 world (solids, logical and physical volumes) based on the GEMC world.
	g4world = new G4World(gworld, gopt);

	// Return the physical volume for the ROOT world volume.
	return g4world->getG4Volume(ROOTWORLDGVOLUMENAME)->getPhysical();
}

void GDetectorConstruction::ConstructSDandField() {
	logSummary("GDetectorConstruction::ConstructSDandField");

	bool touchableVerbosity = (gopt->getVerbosityFor("gsensitivity") >= GVERBOSITY_DETAILS);

	// Create a local map to hold sensitive detectors.
	map<string, GSensitiveDetector *> sensitiveDetectorsMap;

	// Loop over all systems and their volumes.
	for (auto [systemName, gsystem] : *gworld->getSystemsMap()) {
		for (auto [volumeName, gvolume] : *gsystem->getGVolumesMap()) {
			string digitizationName = gvolume->getDigitization();
			string g4name = gvolume->getG4Name();

			// Ensure the Geant4 logical volume exists.
			if (g4world->getG4Volume(g4name) == nullptr) {
				G4cerr << FATALERRORL << " Error: <" << g4name
					   << "> logical volume not built? This should never happen." << G4endl;
				exit(99);
			}

			// Skip volumes with no digitization.
			if (digitizationName != UNINITIALIZEDSTRINGQUANTITY) {
				// Create the sensitive detector if it does not exist yet.
				if (sensitiveDetectorsMap.find(digitizationName) == sensitiveDetectorsMap.end()) {
					logSummary("Sensitive detector <" + digitizationName + "> doesn't exist for <" + g4name + ">. Creating it.");
					sensitiveDetectorsMap[digitizationName] = new GSensitiveDetector(digitizationName, gopt, gDynamicDigitizationMapGlobalInstance);
					auto sdManager = G4SDManager::GetSDMpointer();
					sdManager->SetVerboseLevel(10);
					sdManager->AddNewDetector(sensitiveDetectorsMap[digitizationName]);
				} else {
					logDetail("Sensitive detector <" + digitizationName + "> exists for <" + volumeName + ">");
				}

				// Register the volume touchable with the sensitive detector.
				sensitiveDetectorsMap[digitizationName]->registerGVolumeTouchable(
						g4name,
						new GTouchable(digitizationName, gvolume->getGIdentity(), gvolume->getDetectorDimensions(), touchableVerbosity)
				);
				SetSensitiveDetector(g4name, sensitiveDetectorsMap[digitizationName]);
			}

			// Process electromagnetic fields.
			string field_name = gvolume->getEMField();
			if (field_name != UNINITIALIZEDSTRINGQUANTITY) {
				if (gmagneto == nullptr) {
					gmagneto = new GMagneto(gopt);
				}
				logDetail("Volume <" + volumeName + "> has field: <" + field_name + ">. Looking into field map definitions.");
				logDetail("Setting field manager for volume <" + g4name + "> with field <" + field_name + ">");
				g4world->setFieldManagerForVolume(g4name, gmagneto->getFieldMgr(field_name), true);
			}
		}
	}

	// Load digitization plugins after constructing sensitive detectors.
	loadDigitizationPlugins();
}

void GDetectorConstruction::loadDigitizationPlugins() {
	vector<string> sdetectors = gworld->getSensitiveDetectorsList();
	int verbosity = gopt->getVerbosityFor("gsensitivity");

	for (auto &sdname : sdetectors) {
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
				cout << "Loading plugins from file " << sdname << endl;
			}
			GManager sdPluginManager(sdname + " GSensitiveDetector", verbosity);
			if (gDynamicDigitizationMapGlobalInstance->find(sdname) == gDynamicDigitizationMapGlobalInstance->end()) {
				(*gDynamicDigitizationMapGlobalInstance)[sdname] = sdPluginManager.LoadAndRegisterObjectFromLibrary<GDynamicDigitization>(sdname);
				(*gDynamicDigitizationMapGlobalInstance)[sdname]->defineReadoutSpecs();
			}
			// Optionally clear the plugin manager's DL map:
			// sdPluginManager.clearDLMap();
		}
	}
}

void GDetectorConstruction::reload_geometry(vector<GSystem> gs) {
	// Use vector assignment to update the local systems.
	gsystems = gs;
	// Reconstruct the geometry and update the world volume.
	G4RunManager::GetRunManager()->DefineWorldVolume(Construct());
}
