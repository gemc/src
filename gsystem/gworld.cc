// glibrary
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

// c++
using std::cerr;
using std::endl;

GWorld::GWorld(GOptions *gopts) {

    gsystemsMap = new map<string, GSystem *>;

    int verbosity = gopts->getVerbosityFor("gsystem");

    // loading gsystemsMap with GSystems
    for (auto &gsystem: gsystem::getSystems(gopts) ) {
        string keyName = gutilities::getFileFromPath(gsystem.getName());
        (*gsystemsMap)[keyName] = new GSystem(gsystem);
    }

    // loading gmodifiersMap
    for (auto &gmodifier: gsystem::getModifiers(gopts) ) {
        if (gmodifier.getName() != GSYSTEMNOMODIFIER) {
            gmodifiersMap[gmodifier.getName()] = &gmodifier;
        }
    }

    // instantiating gSystemManager and systemFactory
    GManager gSystemManager("GSystemManager", verbosity);

    map < string, GSystemFactory * > systemFactory;

    // registering factories in gSystemManager and adding them to systemFactory
    // if a factory is not existing already, registering it in the manager, instantiating it, and loading it into the map

    // text factory created no matter what, needed to create ROOT volume
    gSystemManager.RegisterObjectFactory<GSystemTextFactory>(GSYSTEMTEXTFACTORYLABEL);

    if (systemFactory.find(GSYSTEMTEXTFACTORYLABEL) == systemFactory.end()) {
        systemFactory[GSYSTEMTEXTFACTORYLABEL] = gSystemManager.CreateObject<GSystemFactory>(GSYSTEMTEXTFACTORYLABEL);
    }

    for (auto &[gsystemName, gsystem]: *gsystemsMap) {

        string factoryName = gsystem->getFactoryName();

        if (factoryName == GSYSTEMCADTFACTORYLABEL) {
            if (systemFactory.find(factoryName) == systemFactory.end()) {
                gSystemManager.RegisterObjectFactory<GSystemCADFactory>(factoryName);
                systemFactory[factoryName] = gSystemManager.CreateObject<GSystemFactory>(factoryName);
            }
        } else if (factoryName == GSYSTEMGDMLTFACTORYLABEL) {
            if (systemFactory.find(factoryName) == systemFactory.end()) {
                gSystemManager.RegisterObjectFactory<GSystemGDMLFactory>(factoryName);
                systemFactory[factoryName] = gSystemManager.CreateObject<GSystemFactory>(factoryName);
            }
        } else if (factoryName == GSYSTEMSQLITETFACTORYLABEL) {
            if (systemFactory.find(factoryName) == systemFactory.end()) {
                gSystemManager.RegisterObjectFactory<GSystemSQLiteFactory>(factoryName);
                systemFactory[factoryName] = gSystemManager.CreateObject<GSystemFactory>(factoryName);
            }
        }
    }

    // done with gSystemManager
    gSystemManager.clearDLMap();

    // now loading gvolumes definitions for all systems
    for (auto &[gsystemName, gsystem]: *gsystemsMap) {
        string factory = gsystem->getFactoryName();

        if (systemFactory.find(factory) != systemFactory.end()) {
            for ( auto yaml_file: gopts->get_yaml_files() ) {
                systemFactory[factory]->addPossibleFileLocation(getDirFromPath(yaml_file));
            }
            systemFactory[factory]->loadSystem(gsystem, verbosity);
            systemFactory[factory]->closeSystem();
        } else {
            cerr << FATALERRORL << "systemFactory factory <" << factory << "> not found for " << gsystemName << endl;
            gexit(EC__FACTORYNOTFOUND);
        }

        // PRAGMA TODO: log accounting
        // account number of volume definitions loaded
    }

    if (verbosity == GVERBOSITY_DETAILS) {
        for (auto &[gsystemName, gsystem]: *gsystemsMap) {
            // first collect all volume names
            for (auto &[volumeName, gvolume]: *gsystem->getGVolumesMap()) {
                cout << GSYSTEMLOGHEADER << "System <" << gsystemName << "> volume " << volumeName << endl;
            }
        }
    }


    // adding root volume to the "root" gsystem
    // using the text factory
    string worldVolumeDefinition = gopts->getScalarString(ROOTWORLDGVOLUMENAME);

    (*gsystemsMap)[ROOTWORLDGVOLUMENAME] = new GSystem(ROOTWORLDGVOLUMENAME, GSYSTEMTEXTFACTORYLABEL, "default", verbosity);
    (*gsystemsMap)[ROOTWORLDGVOLUMENAME]->addROOTVolume(worldVolumeDefinition);

    // applying gvolumes modifiers
    for (auto &[volumeNameToModify, gmodifier]: gmodifiersMap) {

        // looping over systems, searching for volume
        GVolume *thisVolume = searchForVolume(volumeNameToModify, " is marked for modifications ");

        if (thisVolume != nullptr) {
            thisVolume->applyShift(gmodifier->getShift());
            thisVolume->applyTilt(gmodifier->getTilts());
            thisVolume->modifyExistence(gmodifier->getExistence());
        }
    }


    // making sure every detector mother is defined
    for (auto system: *gsystemsMap) {
        // first collect all volume names
        for (auto &[volumeName, gvolume]: *system.second->getGVolumesMap()) {
            // will exit with error if not found
            // skipping world volume
            string motherVolumeName = gvolume->getMotherName();
            if (motherVolumeName != MOTHEROFUSALL) {

                auto motherVolume = searchForVolume(motherVolumeName, "mother of <" + gvolume->getName() + ">");

                string g4name = gvolume->getSystem() + GSYSTEM_DELIMITER + volumeName;
                string g4motherName = motherVolume->getSystem() + GSYSTEM_DELIMITER + motherVolumeName;

                if (motherVolumeName == ROOTWORLDGVOLUMENAME) {
                    g4motherName = ROOTWORLDGVOLUMENAME;
                }

                gvolume->assignG4Names(g4name, g4motherName);

            } else {
                gvolume->assignG4Names(ROOTWORLDGVOLUMENAME, MOTHEROFUSALL);

            }
        }
    }

    // PRAGMA TODO: Loads material


}


// seerch for a volume among systems in gsystemsMap
GVolume *GWorld::searchForVolume(string volumeName, string purpose) const {

    for (auto &system: *gsystemsMap) {

        GVolume *thisVolume = system.second->getGVolume(volumeName);
        if (thisVolume != nullptr) {
            return thisVolume;
        }
    }

    // error: volume not found
    cerr << FATALERRORL << "gvolume named <" << volumeName << "> (" << purpose << ") not found in gsystemsMap " << endl;
    gexit(EC__GVOLUMENOTFOUND);

    return nullptr;
}


vector <string> GWorld::getSensitiveDetectorsList() {
    vector <string> snames;
    for (auto &system: *gsystemsMap) {
        for (auto &gvolume: *system.second->getGVolumesMap()) {
            string digitization = gvolume.second->getDigitization();
            if (digitization != UNINITIALIZEDSTRINGQUANTITY) {
                if (find(snames.begin(), snames.end(), digitization) == snames.end())
                    snames.push_back(digitization);
            }
        }
    }

    return snames;
}



// PRAGMA TODO: Loads materials
