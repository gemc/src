// gdetectorConstruction
#include "gdetectorConstruction.h"

// gemc
#include "gtouchableConventions.h"
#include "ginternalDigitization.h"
#include "gsystemConventions.h"

// geant4
#include "G4SDManager.hh"
#include "G4GeometryManager.hh"
#include "G4SolidStore.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4ReflectionFactory.hh"
#include "G4RunManager.hh"
#include "gdynamicdigitizationConventions.h"

G4ThreadLocal std::unique_ptr<GMagneto> GDetectorConstruction::gmagneto = nullptr;

GDetectorConstruction::GDetectorConstruction(std::shared_ptr<GOptions>                           gopts,
                                             std::shared_ptr<gdynamicdigitization::dRoutinesMap> digiRoutinesMap)
	: G4VUserDetectorConstruction(), // Geant4 base class.
	  gopt(gopts),                   // need this in Construct and ConstructSDandField
	  log(std::make_shared<GLogger>(gopts, "GDetectorConstruction", "GDetectorConstruction")),
	  digitizationRoutinesMap(digiRoutinesMap) { log->debug(CONSTRUCTOR, "GDetectorConstruction"); }

GDetectorConstruction::~GDetectorConstruction() {
	// Clean up the GEMC and Geant4 world objects.
	log->debug(DESTRUCTOR, "GDetectorConstruction");
}

G4VPhysicalVolume* GDetectorConstruction::Construct() {
	// Clean any old geometry.
	G4GeometryManager::GetInstance()->OpenGeometry();
	G4PhysicalVolumeStore::Clean();
	G4LogicalVolumeStore::GetInstance()->Clean();
	G4SolidStore::GetInstance()->Clean();
	G4ReflectionFactory::Instance()->Clean();

	// Delete old geometry objects if they exist
	gworld.reset();
	g4world.reset();

	// - if no systems are provided, we just launched gemc: create from options
	// - otherwise, it's a geometry re-load. use existing systems.
	if (gsystems.empty()) { gworld = std::make_shared<GWorld>(gopt.get()); }
	else { gworld = std::make_shared<GWorld>(gopt.get(), std::move(gsystems)); }

	// Build Geant4 world (solids, logical and physical volumes) based on the GEMC world.
	g4world = std::make_shared<G4World>(gworld.get(), gopt.get());

	// Return the physical volume for the ROOT world volume.
	return g4world->getG4Volume(ROOTWORLDGVOLUMENAME)->getPhysical();
}

void GDetectorConstruction::ConstructSDandField() {
	log->debug(NORMAL, "GDetectorConstruction::ConstructSDandField");

	// Loop over all systems and their volumes.
	for (const auto& [systemName, gsystemPtr] : *gworld->getSystemsMap()) {
		for (const auto& [volumeName, gvolumePtr] : gsystemPtr->getGVolumesMap()) {
			auto const& digitizationName = gvolumePtr->getDigitization();
			auto const& g4name           = gvolumePtr->getG4Name();

			// Ensure the Geant4 logical volume exists.
			if (g4world->getG4Volume(g4name) == nullptr) {
				log->error(ERR_GVOLUMENOTFOUND, "GDetectorConstruction::ConstructSDandField", "Logical volume <" + g4name + "> not found.");
			}

			// Skip volumes with no digitization.
			if (digitizationName != UNINITIALIZEDSTRINGQUANTITY) {
				// Create the sensitive detector if it does not exist yet.
				if (sensitiveDetectorsMap.find(digitizationName) == sensitiveDetectorsMap.end()) {
					log->info(2, "Creating new sensitive detector <" + digitizationName + "> for volume <" + g4name + ">");
					sensitiveDetectorsMap[digitizationName] = std::make_shared<GSensitiveDetector>(digitizationName, gopt, digitizationRoutinesMap);

					auto sdManager = G4SDManager::GetSDMpointer();
					sdManager->SetVerboseLevel(10);
					sdManager->AddNewDetector(sensitiveDetectorsMap[digitizationName].get());
				}
				else { log->info(2, "Sensitive detector <" + digitizationName + "> is already created and available for volume <" + g4name + ">"); }

				// Register the volume touchable with the sensitive detector
				const auto& vdimensions     = gvolumePtr->getDetectorDimensions();
				const auto& identity        = gvolumePtr->getGIdentity();
				auto        this_gtouchable = std::make_shared<GTouchable>(digitizationName, identity, vdimensions, log);
				sensitiveDetectorsMap[digitizationName]->registerGVolumeTouchable(g4name, this_gtouchable);
				SetSensitiveDetector(g4name, sensitiveDetectorsMap[digitizationName].get());
			}

			// Process electromagnetic fields.
			const auto& field_name = gvolumePtr->getEMField();
			if (field_name != UNINITIALIZEDSTRINGQUANTITY) {
				if (gmagneto == nullptr) { gmagneto = std::make_unique<GMagneto>(gopt); }
				log->info(2, "Volume <" + volumeName + "> has field: <" + field_name + ">. Looking into field map definitions.");
				log->info(2, "Setting field manager for volume <" + g4name + "> with field <" + field_name + ">");
				g4world->setFieldManagerForVolume(g4name, gmagneto->getFieldMgr(field_name).get(), true);
			}
		}
	}

	// Load digitization plugins after constructing sensitive detectors.
	loadDigitizationPlugins();


	// tally with number :
	log->info(0, "Tally summary: \n - ", gworld->get_number_of_volumes(), " volumes\n - ",
	          g4world->number_of_volumes(), "geant4 built volumes");
}

void GDetectorConstruction::loadDigitizationPlugins() {
	const auto sdetectors = gworld->getSensitiveDetectorsList();

	for (auto& sdname : sdetectors) {
		if (sdname == FLUXNAME) {
			log->info(1, "Loading flux digitization plugin for routine <" + sdname + ">");
			digitizationRoutinesMap->emplace(sdname, std::make_shared<GFluxDigitization>());
		}
		else if (sdname == COUNTERNAME) {
			log->info(1, "Loading particle counter digitization plugin for routine <" + sdname + ">");
			digitizationRoutinesMap->emplace(sdname, std::make_shared<GParticleCounterDigitization>());
		}
		else if (sdname == DOSIMETERNAME) {
			log->info(1, "Loading dosimeter digitization plugin for routine <" + sdname + ">");
			digitizationRoutinesMap->emplace(sdname, std::make_shared<GDosimeterDigitization>());
		}
		else {
			// if it's not in the map already, add it
			if (digitizationRoutinesMap->find(sdname) == digitizationRoutinesMap->end()) {
				log->info(0, "Loading new digitization plugin for routine <" + sdname + ">");
				digitizationRoutinesMap->emplace(sdname, gdynamicdigitization::load_dynamicRoutine(sdname, gopt.get()));
			}
		}

		if ( digitizationRoutinesMap->at(sdname)->defineReadoutSpecs() ) {
			log->info(1, "Digitization routine <" + sdname + "> has been successfully defined.");
		} else {
			log->error(ERR_DEFINESPECFAIL, "defineReadoutSpecs failure for <" + sdname + ">");
		}
	}
}


void GDetectorConstruction::reload_geometry(SystemList&& sl) {
	// Use vector assignment to update the local systems.
	gsystems = std::move(sl);
	// Reconstruct the geometry and update the world volume.
	G4RunManager::GetRunManager()->DefineWorldVolume(Construct());
}
