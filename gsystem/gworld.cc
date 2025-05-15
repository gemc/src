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


GWorld::GWorld(GOptions* g)
    : gopts(g),
      log(std::make_shared<GLogger>(g, GSYSTEM_LOGGER, "GWorld [new]")) {
    log->debug(CONSTRUCTOR, "GWorld");

    //------------------------------------------------------------------
    // 1.  Load every GSystem and move it into the map
    //------------------------------------------------------------------
    auto systems = gsystem::getSystems(gopts, log);   // keeps vector alive

    for (auto& sysPtr : systems) {                    // unique_ptr<GSystem>&
        std::string key = gutilities::getFileFromPath(sysPtr->getName());

        gsystemsMap->emplace(key, std::move(sysPtr)); // transfer ownership
    }
    systems.clear();   // optional: emphasise vector is now empty

    //------------------------------------------------------------------
    // 2.  Finish world construction
    //------------------------------------------------------------------
    load_systems();     // build factories, load volumes
    load_gmodifiers();  // load & apply modifiers
    assignG4Names();    // final bookkeeping
}


GWorld::GWorld(GOptions* g, SystemList systems)
	: gopts(g),
	  gsystemsMap(std::make_unique<SystemMap>()),
	  log(std::make_shared<GLogger>(g, GSYSTEM_LOGGER, "GWorld [update]")) {
	//------------------------------------------------------------------
	// Move each unique_ptr<GSystem> from the vector into the map
	//------------------------------------------------------------------
	for (auto& sysPtr : systems) { // sysPtr is a UNIQUE_PTR&
		std::string key =
			gutilities::getFileFromPath(sysPtr->getName());

		// transfer ownership; sysPtr becomes nullptr
		(*gsystemsMap)[key] = std::move(sysPtr);
	}
	systems.clear(); // optional: make intent explicit

	//------------------------------------------------------------------
	// Finish world construction
	//------------------------------------------------------------------
	load_systems();    // instantiate factories, load volumes
	load_gmodifiers(); // load modifiers
	assignG4Names();   // apply modifiers & set G4 names
}

GWorld::~GWorld() { log->debug(DESTRUCTOR, "GWorld"); }

/**
 * createSystemFactory creates a local GManager, registers the required system factories,
 * clears its DL map, and returns a pointer to a map of factory names to GSystemFactory pointers.
 */
std::map<std::string, std::unique_ptr<GSystemFactory>> GWorld::createSystemFactory(SystemMap* gsystemsMap) {
    //----------------------------------------------------------------------
    // 0.  Local RAII manager
    //----------------------------------------------------------------------
    GManager manager(log, "GWorldManager");

    std::map<std::string, std::unique_ptr<GSystemFactory>> factoryMap;

    //----------------------------------------------------------------------
    // 1.  Always register & create the SQLite factory (needed for ROOT volumes)
    //----------------------------------------------------------------------
    manager.RegisterObjectFactory<GSystemSQLiteFactory>(GSYSTEMSQLITETFACTORYLABEL);

    auto sqliteFactory = std::unique_ptr<GSystemFactory>(
        manager.CreateObject<GSystemFactory>(GSYSTEMSQLITETFACTORYLABEL));

    if (!sqliteFactory) {
        log->error(ERR_FACTORYNOTFOUND,
                   "Failed to create factory <", GSYSTEMSQLITETFACTORYLABEL, ">");
    }
    factoryMap.emplace(GSYSTEMSQLITETFACTORYLABEL, std::move(sqliteFactory));

    //----------------------------------------------------------------------
    // 2.  Scan all systems and create any missing factories
    //----------------------------------------------------------------------
    for (auto& [sysName, sysPtr] : *gsystemsMap) {
        const std::string& facName = sysPtr->getFactoryName();

        if (facName.empty()) {
            log->error(ERR_FACTORYNOTFOUND,
                       "Factory name for system <", sysName,
                       "> is empty!  This system will not be loaded.");
        }

        // Already have it?  Move on.
        if (factoryMap.count(facName)) continue;

        //------------------ register the correct concrete class ----------------
        if      (facName == GSYSTEMCADTFACTORYLABEL)
            manager.RegisterObjectFactory<GSystemCADFactory>(facName);
        else if (facName == GSYSTEMGDMLTFACTORYLABEL)
            manager.RegisterObjectFactory<GSystemGDMLFactory>(facName);
        else if (facName == GSYSTEMSQLITETFACTORYLABEL)
            manager.RegisterObjectFactory<GSystemSQLiteFactory>(facName);
        else if (facName == GSYSTEMASCIIFACTORYLABEL)
            manager.RegisterObjectFactory<GSystemTextFactory>(facName);
        else {
            log->error(ERR_FACTORYNOTFOUND,
                       "Unrecognized factory name <", facName,
                       "> for system <", sysName, ">");
        }

        //------------------ create the factory object --------------------------
        auto facPtr = std::unique_ptr<GSystemFactory>(
            manager.CreateObject<GSystemFactory>(facName));

        if (!facPtr) {
            log->error(ERR_FACTORYNOTFOUND,
                       "Failed to create factory <", facName,
                       "> for system <", sysName, ">");
        }

        factoryMap.emplace(facName, std::move(facPtr));
    }

    //----------------------------------------------------------------------
    // 3.  Clean up any temporarily loaded shared libraries
    //----------------------------------------------------------------------
    manager.clearDLMap();

    // 4.  Return by value (NRVO/move) – no leaks, no manual delete
    return factoryMap;
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
	//------------------------------------------------------------------
	// 0.  Setup
	//------------------------------------------------------------------
	const std::string dbhost = gopts->getScalarString("sql");

	/*  createSystemFactory() now RETURNS the map by value:
	 *      std::map<std::string,std::unique_ptr<GSystemFactory>>
	 *  NRVO / move makes this cheap.  No raw `new`, no manual delete.
	 */
	auto systemFactories = createSystemFactory(gsystemsMap.get());

	//------------------------------------------------------------------
	// 1.  Inject the ROOT “world” volume, if not already present
	//------------------------------------------------------------------
	const std::string worldVolumeDefinition =
		gopts->getScalarString(ROOTWORLDGVOLUMENAME);

	if (gsystemsMap->count(ROOTWORLDGVOLUMENAME)) {
		log->error(ERR_GVOLUMEALREADYPRESENT,
		           "ROOT world volume already exists in gsystemsMap. "
		           "Check your configuration.");
	}
	else {
		auto rootSystem = std::make_unique<GSystem>(
		                                            log, // logger
		                                            dbhost,
		                                            ROOTWORLDGVOLUMENAME, // name + path
		                                            GSYSTEMSQLITETFACTORYLABEL,
		                                            "all",    // experiment
		                                            1,        // runNo
		                                            "default" // variation
		                                           );
		rootSystem->addROOTVolume(worldVolumeDefinition);

		(*gsystemsMap)[ROOTWORLDGVOLUMENAME] = std::move(rootSystem);
	}

	//------------------------------------------------------------------
	// 2.  For every system, find / create its factory and load volumes
	//------------------------------------------------------------------
	const auto yamlFiles = gopts->getYamlFiles();

	for (auto& [sysName, sysPtr] : *gsystemsMap) {
		const std::string& factoryName = sysPtr->getFactoryName();

		if (factoryName.empty()) {
			log->error(ERR_FACTORYNOTFOUND,
			           "Factory name for system <", sysName, "> is empty!");
		}

		auto facIt = systemFactories.find(factoryName);
		if (facIt == systemFactories.end()) {
			log->error(ERR_FACTORYNOTFOUND,
			           "Factory <", factoryName, "> not found for system <", sysName, ">");
		}

		auto& factory = facIt->second; // std::unique_ptr<GSystemFactory>&
		if (!factory) {
			log->error(ERR_FACTORYNOTFOUND,
			           "Factory pointer <", factoryName, "> is nullptr");
		}

		// Feed YAML directories as possible file locations
		for (const auto& yaml : yamlFiles) {
			std::string dir = gutilities::getDirFromPath(yaml);
			if (dir.empty())
				log->warning("Directory extracted from YAML <", yaml, "> is empty.");
			factory->addPossibleFileLocation(dir);
		}

		// Load & close the system
		factory->loadSystem(sysPtr.get(), log);
		factory->closeSystem(log);
	}

	//------------------------------------------------------------------
	// 3.  Optional verbose dump of every volume
	//------------------------------------------------------------------
	for (auto& [sysName, sysPtr] : *gsystemsMap)
		for (auto& [volName, volPtr] : sysPtr->getGVolumesMap())
			log->info(2, *volPtr); // level‑2 info

	// systemFactories goes out of scope -> all factories destroyed cleanly
}


/**
 * load_gmodifiers loads the GModifier objects into the modifier map and applies modifications.
 */
void GWorld::load_gmodifiers() {
	//------------------------------------------------------------------
	// 1.  Build the map <volumeName → unique_ptr<GModifier>>
	//------------------------------------------------------------------
	for (const auto& mod : gsystem::getModifiers(gopts))               // returns vector<GModifier>
	{
		// Deep‑copy each modifier into a unique_ptr and move into the map
		auto modPtr = std::make_unique<GModifier>(mod);
		gmodifiersMap.emplace(modPtr->getName(), std::move(modPtr));
	}

	//------------------------------------------------------------------
	// 2.  Apply every modifier to its target volume
	//------------------------------------------------------------------
	for (auto& [volumeName, modPtr] : gmodifiersMap)                   // modPtr is unique_ptr<GModifier>&
	{
		// Will exit if not found:
		GVolume* vol = searchForVolume(volumeName,
									   " is marked for modifications");

		vol->applyShift   (modPtr->getShift());
		vol->applyTilt    (modPtr->getTilts());
		vol->modifyExistence(modPtr->getExistence());

		log->info(2, "g‑modifying volume <", volumeName,
				  "> with modifier: ", *modPtr);
		log->info(2, "After modifications:", *vol);
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
