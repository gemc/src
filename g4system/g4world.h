#pragma once
/**
 * @file   g4world.h
 * @ingroup Geometry
 * @brief  Builds the complete Geant4 “world” from a GEMC `GWorld`.
 *
 * A **G4World** orchestrating:
 *  1.  Loading materials into the Geant4 NIST table.
 *  2.  Converting every `GVolume` into a `G4Volume` through factories
 *      (`G4NativeSystemFactory`, `G4CadSystemFactory`, …).
 *  3.  Maintaining maps so later stages (e.g., field assignment) can look
 *      volumes up by name.
 *
 * All pointers inside the maps are *owned by Geant4* once registered; this
 * wrapper merely tracks them.
 */

// gemc
#include "gworld.h"

// g4system
#include "g4volume.h"
#include "g4objectsFactories/g4objectsFactory.h"
#include "g4system/g4systemConventions.h"

// c++
#include <memory>
#include <string>
#include <unordered_map>

// geant4
#include "G4NistManager.hh"

/**
 * @class G4World
 * @brief High‑level builder that turns a GEMC `GWorld` into Geant4 geometry.
 *
 * Public helpers let clients:
 *  * query a `G4Volume*` by name,
 *  * attach a `G4FieldManager` to a whole volume subtree,
 *  * ask how many volumes were produced.
 */
class G4World {
public:
	/**
	 * @brief Construct the world from a GEMC world description.
	 * @param gworld Pointer to the GEMC world object.
	 * @param gopts  Global options (verbosity, backup material, overlaps…).
	 */
	G4World(const GWorld* gworld, GOptions* gopts);

	/** Destructor – shutdown; no manual deletion needed. */
	~G4World() { log->debug(DESTRUCTOR, "G4World"); }

	// ────── lookup / mutators ────────────────────────────────────────
	/**
	 * @brief Return the `G4Volume*` for @p volumeName or `nullptr`.
	 */
	[[nodiscard]] const G4Volume* getG4Volume(const std::string& volumeName) const;

	/**
	 * @brief Attach a `G4FieldManager` to the logical volume of @p volumeName.
	 * @param volumeName           Name of the volume.
	 * @param fm                    Field manager pointer.
	 * @param forceToAllDaughters   Propagate to daughters.
	 */
	void setFieldManagerForVolume(const std::string& volumeName,
	                              G4FieldManager*    fm,
	                              bool               forceToAllDaughters);

	/**
	 * @brief Check if no volumes were built.
	 */
	[[nodiscard]] bool is_empty() const noexcept { return g4volumesMap.empty(); }

	/**
	 * @brief Number of volumes stored so far.
	 */
	[[nodiscard]] std::size_t number_of_volumes() const noexcept { return g4volumesMap.size(); }

	[[nodiscard]] G4ObjectsFactory* get_factory(const std::string& factoryName) {
		auto it = g4systemFactory.find(factoryName);
		if (it != g4systemFactory.end()) return it->second;
		else {
			log->error(ERR_G4SYSTEMFACTORYNOTFOUND,
			           "G4World: factory <", factoryName, "> not found.");
		}
	}

private:
	// ────── helper functions ─────────────────────────────────────────
	std::string g4FactoryNameFromSystemFactory(const std::string& factory) const;

	bool createG4Material(const std::unique_ptr<GMaterial>& gmaterial);
	void buildDefaultMaterialsElementsAndIsotopes();
	void buildMaterials(SystemMap* system_map);

	// dependency helpers
	G4Element* Deuterium = nullptr;
	G4Element* Helium3   = nullptr;
	G4Element* Tritium   = nullptr;

	// ────── data members ────────────────────────────────────────────
	std::unordered_map<string, G4ObjectsFactory*> g4systemFactory;

	/** Map "gsystem/volumeName" → G4Volume*. */
	std::unordered_map<std::string, G4Volume*> g4volumesMap;

	/**
	 * Map "materialName" → `G4Material*`.
	 * The sole purpose is to keep the pointer around for later queries.
	 */
	std::unordered_map<std::string, G4Material*> g4materialsMap;

	/** Shared logger (verbosity controlled via `GOptions`). */
	std::shared_ptr<GLogger> log;

	/**
	 * Creates and initializes the system factory map.
	 *
	 * This method internally creates a GManager, registers the required factories,
	 * clears the DL map, and returns a pointer to a map (factory name → GSystemFactory pointer).
	 *
	 * @param gsystemsMap Pointer to the map of GSystem objects.
	 * @param backup_material Name of the backup material to use if the requested one is missing.
	 * @param check_overlaps Flag to enable or disable overlap checking.
	 * @return Pointer to the created system factory map.
	 */
	void createG4SystemFactory(SystemMap*    gsystemsMap,
	                           const string& backup_material,
	                           int           check_overlaps);

	bool build_g4volume(const GVolume* s, G4ObjectsFactory* objectFactory);

};
