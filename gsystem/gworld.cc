// gemc
#include "gfactory.h"
#include "gutilities.h"

// gsystem
#include "gworld.h"
#include "gmodifier.h"
#include "gsystemFactories/systemFactory.h"
#include "gsystemFactories/text/systemTextFactory.h"
#include "gsystemFactories/cad/systemCadFactory.h"
#include "gsystemFactories/gdml/systemGdmlFactory.h"
#include "gsystemFactories/sqlite/systemSqliteFactory.h"


GWorld::GWorld(GOptions* g) : gopts(g), log(std::make_shared<GLogger>(g, GSYSTEM_LOGGER)) {
	log->debug(CONSTRUCTOR, "GWorld");

	// Allocate the systems map.
	gsystemsMap   = new map<string, GSystem*>;
	string dbhost = gopts->getScalarString("sql");

	// Load GSystems into gsystemsMap.
	for (auto& gsystem : gsystem::getSystems(gopts, log)) {
		string keyName          = gutilities::getFileFromPath(gsystem.getName());
		(*gsystemsMap)[keyName] = new GSystem(gsystem);
	}

	// Create and initialize system factories and load volume definitions.
	load_systems();

	// Load GModifiers into gmodifiersMap.
	load_gmodifiers();

	// Apply volume modifiers and assign G4 names.
	assignG4Names();
}

GWorld::GWorld(GOptions* g, const vector<GSystem>& gsystems) : gopts(g) {
	// Allocate the systems map.
	gsystemsMap = new map<string, GSystem*>;

	// Load GSystems from the provided vector.
	for (auto& gsystem : gsystems) {
		string keyName          = gutilities::getFileFromPath(gsystem.getName());
		(*gsystemsMap)[keyName] = new GSystem(gsystem);
	}

	// Create and initialize system factories and load volume definitions.
	load_systems();

	// Load GModifiers into gmodifiersMap.
	load_gmodifiers();

	// Apply volume modifiers and assign G4 names.
	assignG4Names();
}

GWorld::~GWorld() {
	log->debug(DESTRUCTOR, "GWorld");

	// Delete allocated GSystem objects.
	if (gsystemsMap) {
		for (auto& pair : *gsystemsMap) { delete pair.second; }
		delete gsystemsMap;
	}
	// Delete allocated GModifier objects.
	for (auto& pair : gmodifiersMap) { delete pair.second; }
}

/**
 * createSystemFactory creates a local GManager, registers the required system factories,
 * clears its DL map, and returns a pointer to a map of factory names to GSystemFactory pointers.
 */
map<string, GSystemFactory*>* GWorld::createSystemFactory(map<string, GSystem*>* gsystemsMap) {
	// Create a local GManager.
	GManager manager(log, "GWorld Manager");
	auto     systemFactory = new map<string, GSystemFactory*>;

	// Register and create the default sqlite factory (needed for ROOT volume creation).
	manager.RegisterObjectFactory<GSystemSQLiteFactory>(GSYSTEMSQLITETFACTORYLABEL);
	auto textFactory = manager.CreateObject<GSystemFactory>(GSYSTEMSQLITETFACTORYLABEL);

	if (!textFactory) {
		log->error(EC__FACTORYNOTFOUND, "Failed to create text factory for factory <", GSYSTEMSQLITETFACTORYLABEL, ">");
	}
	(*systemFactory)[GSYSTEMSQLITETFACTORYLABEL] = textFactory;

	// Loop over all systems and register additional factories as needed.
	for (auto& [gsystemName, gsystem] : *gsystemsMap) {
		string factoryName = gsystem->getFactoryName();
		if (factoryName.empty()) {
			log->error(EC__FACTORYNOTFOUND,
			           "Factory name for system <", gsystemName, "> is empty! This system will not be loaded.");
		}
		// If we haven't already created this factory, register it.
		if (systemFactory->find(factoryName) == systemFactory->end()) {
			if (factoryName == GSYSTEMCADTFACTORYLABEL) {
				manager.RegisterObjectFactory<GSystemCADFactory>(factoryName);
			}
			else if (factoryName == GSYSTEMGDMLTFACTORYLABEL) {
				manager.RegisterObjectFactory<GSystemGDMLFactory>(factoryName);
			}
			else if (factoryName == GSYSTEMSQLITETFACTORYLABEL) {
				manager.RegisterObjectFactory<GSystemSQLiteFactory>(factoryName);
			}
			else if (factoryName == GSYSTEMASCIIFACTORYLABEL) {
				manager.RegisterObjectFactory<GSystemTextFactory>(factoryName);
			}
			else {
				log->error(EC__FACTORYNOTFOUND,
				           "Unrecognized factory name <", factoryName, "> for system <", gsystemName, ">");
			}

			auto fac = manager.CreateObject<GSystemFactory>(factoryName);
			if (!fac) {
				log->error(EC__FACTORYNOTFOUND, "Failed to create factory <", factoryName,
				           "> for system <", gsystemName, ">");
			}
			(*systemFactory)[factoryName] = fac;
		}
	}

	// Clear the dynamic library map before returning.
	manager.clearDLMap();
	return systemFactory;
}


GVolume* GWorld::searchForVolume(const string& volumeName, const string& purpose) const {
	for (auto& systemPair : *gsystemsMap) {
		GVolume* thisVolume = systemPair.second->getGVolume(volumeName);
		if (thisVolume != nullptr) {
			log->info(1, "gvolume named <", volumeName, "> found with purpose: ", purpose);
			return thisVolume;
		}
	}
	// If volume not found, print error and exit.
	log->error(ERR_GVOLUMENOTFOUND,
	           "gvolume named <", volumeName, "> (", purpose, ") not found in gsystemsMap ", purpose);
}

vector<string> GWorld::getSensitiveDetectorsList() {
	vector<string> snames;
	for (auto& systemPair : *gsystemsMap) {
		for (auto& gvolumePair : systemPair.second->getGVolumesMap()) {
			string digitization = gvolumePair.second->getDigitization();
			if (digitization != UNINITIALIZEDSTRINGQUANTITY) {
				if (find(snames.begin(), snames.end(), digitization) == snames.end())
					snames.push_back(digitization);
			}
		}
	}
	return snames;
}

/**
 * load_systems creates and initializes system factories and loads volume definitions.
 */
void GWorld::load_systems() {
	string dbhost = gopts->getScalarString("sql");

	// Create system factories.
	map<string, GSystemFactory*>* systemFactory = createSystemFactory(gsystemsMap);

	// Add the ROOT world volume.
	string worldVolumeDefinition = gopts->getScalarString(ROOTWORLDGVOLUMENAME);
	// Check if ROOTWORLDGVOLUMENAME already exists, if so, warn.
	if (gsystemsMap->find(ROOTWORLDGVOLUMENAME) != gsystemsMap->end()) {
		log->error(ERR_GVOLUMEALREADYPRESENT,
		           "ROOT world volume already exists in gsystemsMap. Check your configuration.");
	}
	else {
		(*gsystemsMap)[ROOTWORLDGVOLUMENAME] = new GSystem(log,
		                                                   dbhost,
		                                                   ROOTWORLDGVOLUMENAME,
		                                                   GSYSTEMSQLITETFACTORYLABEL,
		                                                   "all",
		                                                   1,
		                                                   "default");
		(*gsystemsMap)[ROOTWORLDGVOLUMENAME]->addROOTVolume(worldVolumeDefinition);
	}


	// Log the number of YAML files available.
	auto yamlFiles = gopts->getYamlFiles();

	// Loop over all systems in the map.
	for (auto& [gsystemName, gsystem] : *gsystemsMap) {
		string factory = gsystem->getFactoryName();
		if (factory.empty()) {
			log->error(EC__FACTORYNOTFOUND, "Factory name for system <", gsystemName, "> is empty!");
		}
		if (systemFactory->find(factory) != systemFactory->end()) {
			// For each YAML file, add its directory to the possible file locations.
			for (auto& yaml_file : yamlFiles) {
				string dir = gutilities::getDirFromPath(yaml_file);
				// You might add a check that 'dir' is nonempty.
				if (dir.empty()) { log->warning("Directory extracted from YAML file <", yaml_file, "> is empty."); }
				systemFactory->at(factory)->addPossibleFileLocation(dir);
			}

			// Load the system.
			if (systemFactory->at(factory) != nullptr) {
				systemFactory->at(factory)->loadSystem(gsystem, log);
				systemFactory->at(factory)->closeSystem(log);
			}
			else {
				log->error(EC__FACTORYNOTFOUND,
				           "systemFactory->at(", factory, ") returned nullptr for system <", gsystemName, ">");
			}
		}
		else {
			log->error(EC__FACTORYNOTFOUND,
			           "systemFactory factory <", factory, "> not found for system <", gsystemName, ">");
		}
	}

	// Optionally log volume definitions.
	for (auto& [gsystemName, gsystem] : *gsystemsMap) {
		for (auto& [volumeName, gvolume] : gsystem->getGVolumesMap()) { log->info(2, *gvolume); }
	}


	// Clean up the local systemFactory.
	delete systemFactory;
}


/**
 * load_gmodifiers loads the GModifier objects into the modifier map and applies modifications.
 */
void GWorld::load_gmodifiers() {
	// Load GModifiers into gmodifiersMap.
	for (auto& gmodifier : gsystem::getModifiers(gopts)) {
		gmodifiersMap[gmodifier.getName()] = new GModifier(gmodifier);
	}

	// Apply modifiers to the volumes.
	for (auto& [volumeNameToModify, gmodifier] : gmodifiersMap) {
		// will exit if not found
		auto thisVolume = searchForVolume(volumeNameToModify, " is marked for modifications");
		thisVolume->applyShift(gmodifier->getShift());
		thisVolume->applyTilt(gmodifier->getTilts());
		thisVolume->modifyExistence(gmodifier->getExistence());

		log->info(2, "g-modifying volume <", volumeNameToModify, "> with modifier: ", *gmodifier);
		log->info(2, "After modifications:", *thisVolume);
	}
}

/**
 * assignG4Names assigns Geant4 volume names based on the system and volume information.
 */
void GWorld::assignG4Names() {
	for (auto& systemPair : *gsystemsMap) {
		for (auto& [volumeName, gvolume] : systemPair.second->getGVolumesMap()) {
			// Skip if the volume's mother is "MOTHEROFUSALL".
			string motherVolumeName = gvolume->getMotherName();
			if (motherVolumeName != MOTHEROFUSALL) {
				auto   motherVolume = searchForVolume(motherVolumeName, "mother of <" + gvolume->getName() + ">");
				string g4name       = gvolume->getSystem() + GSYSTEM_DELIMITER + volumeName;
				string g4motherName = motherVolume->getSystem() + GSYSTEM_DELIMITER + motherVolumeName;
				if (motherVolumeName == ROOTWORLDGVOLUMENAME) { g4motherName = ROOTWORLDGVOLUMENAME; }
				gvolume->assignG4Names(g4name, g4motherName);
			}
			else { gvolume->assignG4Names(ROOTWORLDGVOLUMENAME, MOTHEROFUSALL); }
		}
	}
}
