// gsystem
#include "gsystem.h"
#include "gsystemConventions.h"

// Use the std namespace in this implementation file.
using namespace std;

// explicit copy constructor
GSystem::GSystem(const GSystem& other)
	: log(other.log),
	  dbhost(other.dbhost),
	  name(other.name),
	  path(other.path),
	  factoryName(other.factoryName),
	  experiment(other.experiment),
	  runno(other.runno),
	  variation(other.variation),
	  annotations(other.annotations)
// shared_ptr: shallow copy is OK
{
	log->debug(CONSTRUCTOR, "GSystem");

	// Deep copy the gvolumesMap
	for (const auto& [key, volumePtr] : other.gvolumesMap) {
		if (volumePtr) {
			gvolumesMap[key] = volumePtr->clone(); // Use the clone method
		}
	}

	// Deep copy the gmaterialsMap
	for (const auto& [key, materialPtr] : other.gmaterialsMap) {
		if (materialPtr) {
			gmaterialsMap[key] = materialPtr->clone(); // Use the clone method
		}
	}
}


/**
 * \file gsystem.cc
 * \brief Implementation of the GSystem class.
 *
 * \details This file contains the definitions for the GSystem class member functions.
 */

GSystem::GSystem(const std::shared_ptr<GLogger>& logger,
                 const std::string&              dbh,
                 const std::string&              sname,
                 const std::string&              factory,
                 const std::string&              exp,
                 int                             run,
                 const std::string&              variation,
                 const std::string&              notes)
	: log(logger),
	  dbhost(dbh),
	  name(gutilities::getFileFromPath(sname)),
	  path(gutilities::getDirFromPath(sname)), // Initialize 'path' directly
	  factoryName(factory),
	  experiment(exp),
	  runno(run),
	  variation(variation),
	  annotations(notes) // Use 'notes' directly
{
	log->debug(CONSTRUCTOR, "GSystem");

	// If the provided name does not include a directory, set the path to empty.
	if (name == path || factoryName == GSYSTEMSQLITETFACTORYLABEL) {
		path = "";
		log->info(1, "Instantiating GSystem <", name, "> using factory <", factoryName, ">");
	}
	else {
		log->info(1, "Instantiating GSystem <", name, "> using path <", path, "> ",
		          "and factory <", factoryName, ">");
	}
}


// MARK: GVOLUMES

/**
 * \brief Builds and adds a GVolume to the system.
 *
 * This function appends the current variation and runs number to the parameters,
 * checks for duplicate volume names, and adds a new GVolume to the gvolumesMap.
 *
 * \param pars A vector of strings containing volume parameters.
 */
void GSystem::addGVolume(vector<string> pars) {
	// Append variation and run number to parameters.
	pars.emplace_back(variation);
	pars.emplace_back(to_string(runno));

	string volume_name = pars[0];

	// Check if the volume already exists in the map.
	if (gvolumesMap.find(volume_name) == gvolumesMap.end()) {
		// Create and add new GVolume to the map.
		gvolumesMap[volume_name] = std::make_unique<GVolume>(log, name, pars);
		log->info(1, "Adding gVolume <" + volume_name + "> to gvolumesMap.");
		log->info(2, *gvolumesMap[volume_name]);
	}
	else {
		log->error(ERR_GVOLUMEALREADYPRESENT,
		           "gVolume <" + volume_name + "> already exists in gvolumesMap.");
	}
}

/**
 * \brief Adds a ROOT volume to the system.
 *
 * This special function constructs a ROOT volume using a dedicated constructor
 * and adds it to the gvolumesMap under the key defined by ROOTWORLDGVOLUMENAME.
 *
 * \param rootVolumeDefinition The definition string for the ROOT volume.
 */
void GSystem::addROOTVolume(const string& rootVolumeDefinition) {
	log->info(1, "Adding ROOT volume <" + rootVolumeDefinition + "> to gvolumesMap.");
	// ROOTWORLDGVOLUMENAME is assumed to be defined in gsystemConventions.h.
	gvolumesMap[ROOTWORLDGVOLUMENAME] = std::make_unique<GVolume>(rootVolumeDefinition);
}

// add volume from a file (CAD or GDML factories)
// includes factory and filename in the definitions
#include <filesystem>
#include <utility>

namespace fs = std::filesystem;

/**
 * \brief Adds a volume from a file (e.g., CAD or GDML).
 *
 * The function splits the file name to extract the volume name and then constructs
 * a vector of parameters (with a specific ordering) to add the volume via addGVolume().
 *
 * \param importType The type of import.
 * \param filename The file that contains the volume definition.
 */
void GSystem::addVolumeFromFile(const string& importType, const string& filename) {
	vector<string> pars;

	// Extract filename (without the path) and split by delimiter.
	vector<string> gvpaths = gutilities::getStringVectorFromStringWithDelimiter(fs::path(filename).filename().string(),
	                                                                            ".");

	// Use the first item as the volume name.
	const string& gvolumeName = gvpaths.front();

	// Order is defined in gvolume.cc:
	// 01: name, 03: type, 04: parameters, 05: material, 02: mother, etc.
	pars.emplace_back(gvolumeName);                 // 01 name
	pars.emplace_back(importType);                  // 03 type
	pars.emplace_back(UNINITIALIZEDSTRINGQUANTITY); // 04 parameters
	pars.emplace_back("G4_AIR");                    // 05 material: default is air
	pars.emplace_back(ROOTWORLDGVOLUMENAME);        // 02 mother: default is ROOTWORLDGVOLUMENAME
	pars.emplace_back("0*cm, 0*cm, 0*cm");          // 06 position
	pars.emplace_back("0*deg, 0*deg, 0*deg");       // 07 rotation
	pars.emplace_back(UNINITIALIZEDSTRINGQUANTITY); // 08 electromagnetic field
	pars.emplace_back("1");                         // 09 visible
	pars.emplace_back("1");                         // 10 style
	pars.emplace_back("999999");                    // 11 color
	pars.emplace_back(UNINITIALIZEDSTRINGQUANTITY); // 12 digitization
	pars.emplace_back(UNINITIALIZEDSTRINGQUANTITY); // 13 gidentity
	pars.emplace_back(UNINITIALIZEDSTRINGQUANTITY); // 14 copyOf
	pars.emplace_back(UNINITIALIZEDSTRINGQUANTITY); // 15 replicaOf
	pars.emplace_back(UNINITIALIZEDSTRINGQUANTITY); // 16 solidsOpr
	pars.emplace_back(UNINITIALIZEDSTRINGQUANTITY); // 17 mirrot
	pars.emplace_back("1");                         // 18 exist
	pars.emplace_back(filename);                    // 19 description: contains full path

	addGVolume(pars);
}


/**
 * \brief Retrieves a pointer to a GVolume given its name.
 *
 * \param volumeName The name of the volume.
 * \return A pointer to the GVolume if it exists, or nullptr otherwise.
 */
GVolume* GSystem::getGVolume(const string& volumeName) const {
	auto it = gvolumesMap.find(volumeName);
	if (it != gvolumesMap.end())
		return it->second.get();
	return nullptr;
}


// MARK: GMATERIALS

/**
 * \brief Adds a GMaterial to the system using a set of parameters.
 *
 * This function checks for duplicate material names before adding a new GMaterial
 * to the gmaterialsMap.
 *
 * \param pars A vector of strings containing material parameters.
 */
void GSystem::addGMaterial(vector<string> pars) {
	string materialName = pars[0];

	if (gmaterialsMap.find(materialName) == gmaterialsMap.end()) {
		gmaterialsMap[materialName] = std::make_unique<GMaterial>(name, pars, log);
		log->info(1, "Adding gMaterial <" + materialName + "> to gmaterialsMap.");
		log->info(2, *gmaterialsMap[materialName]);
	}
	else {
		log->error(ERR_GMATERIALALREADYPRESENT,
		           "gMaterial <" + materialName + "> already exists in gmaterialsMap.");
	}
}

/**
 * \brief Retrieves the material associated with a given volume.
 *
 * The function first retrieves the GVolume from the gvolumesMap and then gets its material name.
 * It then searches the gmaterialsMap for the corresponding material.
 *
 * \param volumeName The name of the volume.
 * \return A pointer to the GMaterial if found, or nullptr otherwise.
 */
const GMaterial* GSystem::getMaterialForGVolume(const string& volumeName) const {
	auto it = gvolumesMap.find(volumeName);
	if (it != gvolumesMap.end()) {
		string materialName = it->second->getMaterial();
		auto   matIt        = gmaterialsMap.find(materialName);
		if (matIt != gmaterialsMap.end())
			return matIt->second.get();
		else {
			log->error(ERR_GMATERIALNOTFOUND,
			           "gMaterial <" + materialName + "> not found for volume <" + volumeName + ">");
		}
	}
	return nullptr;
}

/**
 * \brief Retrieves the full file path of the system.
 *
 * If the path is empty, only the name is returned. Otherwise, the path and name are concatenated.
 *
 * \return A string representing the full file path.
 */
string GSystem::getFilePath() {
	if (path.empty())
		return name;
	return path + "/" + name;
}
