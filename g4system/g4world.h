#pragma once
/**
 * @file   g4world.h
 * @ingroup g4system_geometry
 * @brief  High-level builder that turns a GEMC world description into Geant4 geometry.
 *
 * @details
 * G4World orchestrates the following phases:
 *  1. Create and initialize the Geant4 object factories used by each system.
 *  2. Build materials (including dependency-resolving material composition).
 *  3. Build default materials/elements/isotopes required by common detector configurations.
 *  4. Convert every GVolume into a G4Volume (solid/logical/physical), resolving mother/child dependencies.
 *
 * The built volumes are cached in a map keyed by the Geant4 volume name, so later stages
 * (for example field assignments) can locate volumes by name.
 *
 * @note
 * Geant4 owns most created objects via its internal stores. This class primarily manages
 * construction order and provides lookup helpers.
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
 * @ingroup g4system_geometry
 * @brief Builds a full Geant4 geometry "world" from a GEMC GWorld description.
 *
 * @details
 * Public helpers let clients:
 * - query a G4Volume by name
 * - attach a \c G4FieldManager to a volume subtree (logical volume + daughters)
 * - query whether any volumes were produced
 */
class G4World : public GBase<G4World>
{
public:
	/**
	 * @brief Construct and build the Geant4 world from a GEMC world.
	 *
	 * @param gworld Pointer to the GEMC world object (volume/material definitions).
	 * @param gopts  Global options used by builders and factories (verbosity, overlap checks, backup material).
	 *
	 * @details
	 * The constructor performs the full build:
	 * - initializes factories based on each system factory label
	 * - builds materials and default elements/isotopes
	 * - iteratively builds volumes until all resolvable dependencies are satisfied
	 */
	G4World(const GWorld* gworld, const std::shared_ptr<GOptions>& gopts);

	// ────── lookup / mutators ────────────────────────────────────────

	/**
	 * @brief Return the G4Volume wrapper for a volume name.
	 *
	 * @param volumeName Key used in the internal volume map.
	 * @return Pointer to the cached G4Volume wrapper, or \c nullptr if not found.
	 */
	[[nodiscard]] const G4Volume* getG4Volume(const std::string& volumeName) const;

	/**
	 * @brief Attach a \c G4FieldManager to the logical volume of a named volume.
	 *
	 * @param volumeName          Name of the volume in the internal map.
	 * @param fm                  Field manager pointer to attach.
	 * @param forceToAllDaughters If \c true, propagate to daughter logical volumes.
	 *
	 * @details
	 * If the volume is not found, the method does nothing.
	 * If found but the logical volume is not yet available, the underlying wrapper ignores the request.
	 */
	void setFieldManagerForVolume(const std::string& volumeName,
	                              G4FieldManager*    fm,
	                              bool               forceToAllDaughters);

	/**
	 * @brief Check whether the world currently has no cached volumes.
	 * @return \c true if no volumes were built or cached, otherwise \c false.
	 */
	[[nodiscard]] bool is_empty() const noexcept { return g4volumesMap.empty(); }

	/**
	 * @brief Number of volumes stored so far.
	 * @return Size of the internal volume map.
	 */
	[[nodiscard]] std::size_t number_of_volumes() const noexcept { return g4volumesMap.size(); }

	/**
	 * @brief Retrieve a registered factory by name.
	 *
	 * @param factoryName Factory label used as key in the internal factory map.
	 * @return Pointer to the factory instance.
	 *
	 * @details
	 * If the factory is missing, an error is emitted through the logger.
	 */
	[[nodiscard]] G4ObjectsFactory* get_factory(const std::string& factoryName) {
		auto it = g4systemFactory.find(factoryName);
		if (it != g4systemFactory.end()) return it->second;
		else {
			log->error(ERR_G4SYSTEMFACTORYNOTFOUND,
			           "G4World: factory <", factoryName, "> not found.");
		}
	}

	/**
	 * @brief Return a copy of the current volume map (name → wrapper pointer).
	 * @return Copy of the internal map.
	 *
	 * @note This returns a copy intentionally; callers cannot mutate internal state through it.
	 */
	[[nodiscard]] std::unordered_map<std::string, G4Volume*> get_g4volumes_map() const {
		return g4volumesMap;
	}

private:
	// ────── helper functions ─────────────────────────────────────────

	/**
	 * @brief Map a GEMC system factory label to the corresponding Geant4 object factory label.
	 *
	 * @param factory GEMC system factory label.
	 * @return The Geant4 factory label used to lookup/create a \c G4ObjectsFactory instance.
	 *
	 * @details
	 * The mapping is used when iterating through systems so that each system uses the proper
	 * Geant4 builder implementation (native/CAD/etc).
	 */
	std::string g4FactoryNameFromSystemFactory(const std::string& factory) const;

	/**
	 * @brief Create a Geant4 material from a GEMC material definition if it does not already exist.
	 *
	 * @param gmaterial Shared pointer to the GEMC material definition.
	 * @return \c true if the material exists or was created, \c false if a dependency is still missing.
	 *
	 * @details
	 * Some materials depend on other materials or elements. The method returns \c false when
	 * a required component does not yet exist; the caller can retry later.
	 */
	bool createG4Material(const std::shared_ptr<GMaterial>& gmaterial);

	/**
	 * @brief Create a set of commonly-used isotopes, elements, and materials if missing.
	 *
	 * @details
	 * This method defines a small set of materials used by common detector configurations
	 * (e.g., hydrogen/deuterium/tritium gases and liquids).
	 */
	void buildDefaultMaterialsElementsAndIsotopes();

	/**
	 * @brief Build all materials for all systems, resolving inter-material dependencies.
	 * @param system_map Pointer to the system map holding material definitions.
	 */
	void buildMaterials(SystemMap* system_map);

	/**
	 * @brief Create and initialize all Geant4 object factories required by the provided systems.
	 *
	 * @param gopts           Global options used to configure factory instances.
	 * @param gsystemsMap     Pointer to the system map to inspect.
	 * @param backup_material Backup material name used by factories when a material is missing.
	 * @param check_overlaps  Overlap checking mode forwarded to placement.
	 *
	 * @details
	 * This method registers required factory types in a manager, instantiates them on demand,
	 * and then initializes each with overlap checking and the backup material.
	 */
	void createG4SystemFactory(const std::shared_ptr<GOptions>& gopts,
	                           SystemMap*                       gsystemsMap,
	                           const std::string&               backup_material,
	                           int                              check_overlaps);

	/**
	 * @brief Build the Geant4 representation (solid/logical/physical) for a single GEMC volume.
	 *
	 * @param s              Pointer to the GEMC volume definition to build.
	 * @param objectFactory  Factory instance responsible for the conversion.
	 * @return \c true if solid, logical, and physical volumes were created, otherwise \c false.
	 */
	bool build_g4volume(const GVolume* s, G4ObjectsFactory* objectFactory);

	// ────── dependency helpers ──────────────────────────────────────
	// These pointers are cached so that multiple predefined materials can reuse the same element instances.
	G4Element* Deuterium = nullptr;
	G4Element* Helium3   = nullptr;
	G4Element* Tritium   = nullptr;

	// ────── data members ────────────────────────────────────────────

	/** Map factory label → factory instance pointer. */
	std::unordered_map<std::string, G4ObjectsFactory*> g4systemFactory;

	/** Map "gsystem/volumeName" → G4Volume wrapper pointer. */
	std::unordered_map<std::string, G4Volume*> g4volumesMap;

	/**
	 * @brief Map "materialName" → \c G4Material pointer.
	 *
	 * @details
	 * This map exists to keep pointers available for later queries and diagnostics.
	 * Geant4 stores also maintain ownership/registry of materials.
	 */
	std::unordered_map<std::string, G4Material*> g4materialsMap;
};
