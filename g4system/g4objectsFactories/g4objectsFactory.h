#pragma once
/**
 * @file   g4objectsFactory.h
 * @ingroup Geometry
 * @brief  Abstract factory that converts a GEMC‑DB `GVolume` into Geant4 objects.
 *
 * **Usage: ** derive a concrete builder (`G4NativeSystemFactory`, `G4CadSystemFactory`, …)
 * and implement `buildSolid()`.  You can override `buildLogical()` and
 * `buildPhysical()` if the defaults do not fit your detector geometry.
 */

// c++
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

// gemc
#include "gbase.h"
#include "gvolume.h"

// g4system
#include "g4volume.h"
#include "g4system_options.h"

// geant4
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4RotationMatrix.hh"
#include "G4ThreeVector.hh"
#include "G4VisAttributes.hh"


/**
 * @class G4ObjectsFactory
 * @brief Base class orchestrating the conversion of a `GVolume` into a
 *        *solid*, *logical*, and *physical* Geant4 representation.
 *
 * Call sequence inside `loadG4System()`
 * 1. `buildSolid()`► *pure virtual* –derived class must implement.
 * 2. `buildLogical()` – default uses material lookup, may be overridden.
 * 3. `buildPhysical()` – default places volume, may be overridden.
 *
 * The factory short circuits if dependencies are missing, so systems can be
 * built in any order.
 */
class G4ObjectsFactory : public GBase<G4ObjectsFactory> {
public:
	~G4ObjectsFactory() override = default;

	explicit G4ObjectsFactory(const std::shared_ptr<GOptions>& g) : GBase(g, G4SFACTORY_LOGGER) {
	}


	/**
	 * @brief Provide the runtime logger, overlap‑check flag, and backup material.
	 * @param logger          Shared logger instance (can be reused by many factories).
	 * @param checkOverlaps   Forward to `G4PVPlacement` to enable overlap checking.
	 * @param backupMaterial  Material to fall back on if the requested one is missing;
	 *                        an empty string disables fallback.
	 *
	 * Must be invoked before the first call to `loadG4System()`.
	 */
	void initialize_context(int                checkOverlaps,
	                        const std::string& backupMaterial);

	/**
	 * @brief Build—or retrieve—solid, logical, and physical volumes for @p s.
	 * @param s    Pointer to the GEMC volume definition.
	 * @param g4s  Map that stores the resulting `G4Volume` wrappers.
	 * @return `true` if all three objects were successfully created.
	 */
	[[nodiscard]] bool build_g4volume(const GVolume* s,
	                                  std::unordered_map<std::string,
	                                                     G4Volume*>* g4s);

	/**
	* @brief Short, human‑readable factory name for logging.
	* @return String view valid for the lifetime of the factory.
	*/
	[[nodiscard]] virtual std::string_view className() const = 0;

protected:
	// ──────── pure & virtual hooks for subclasses ──────────────────
	/**
	 * @brief Build the `G4VSolid` for volume @p s.
	 * @note   **Pure‑virtual** – subclasses *must* implement.
	 * @return Pointer to the created solid, or `nullptr` on failure.
	 */
	virtual G4VSolid* buildSolid(const GVolume*                              s,
	                             std::unordered_map<std::string, G4Volume*>* g4s) = 0;

	/**
	 * @brief Build or retrieve the `G4LogicalVolume` for @p s.
	 * @return Pointer to the logical volume, or `nullptr` if dependencies missing.
	 */
	virtual G4LogicalVolume* buildLogical(const GVolume*                              s,
	                                      std::unordered_map<std::string, G4Volume*>* g4s);

	/**
	 * @brief Build or retrieve the `G4VPhysicalVolume` for @p s.
	 * @return Pointer to the physical volume, or `nullptr` if dependencies missing.
	 */
	virtual G4VPhysicalVolume* buildPhysical(const GVolume*                              s,
	                                         std::unordered_map<std::string, G4Volume*>* g4s);


	/// Lookup solid in the `g4s` map.
	static G4VSolid* getSolidFromMap(const std::string& volume_name, std::unordered_map<std::string, G4Volume*>* g4s);

	/// Lookup logical volume in the `g4s` map.
	static G4LogicalVolume* getLogicalFromMap(const std::string& volume_name, std::unordered_map<std::string, G4Volume*>* g4s);

	/// Lookup physical volume in the `g4s` map.
	static G4VPhysicalVolume* getPhysicalFromMap(const std::string& volume_name, std::unordered_map<std::string, G4Volume*>* g4s);

	// get or create a new GVolume in the map
	G4Volume* getOrCreateG4Volume(const std::string& volume_name, std::unordered_map<std::string, G4Volume*>* g4s) {
		if (auto it = g4s->find(volume_name); it != g4s->end()) { return it->second; }
		else {
			(*g4s)[volume_name] = new G4Volume();
			return (*g4s)[volume_name];
		}
	}

	/// Build colour/visibility attributes from `GVolume`.
	G4VisAttributes createVisualAttributes(const GVolume* s);

	/**
	 * @brief Check whether all prerequisites to build a *solid* are satisfied.
	 *
	 * The function handles three mutually exclusive scenarios:
	 * 1. **Copy of** another solid (`copyOf:` syntax)
	 * 2. **Replica of** another solid (future implementation)
	 * 3. **Boolean** solid operations (*addition, subtraction, intersection*)
	 *
	 * @param s    Pointer to the <i>candidate</i> volume.
	 * @param g4s  Map containing every G4Volume created so far.
	 * @return `true` if all dependencies are already present, otherwise `false`.
	 *
	 */
	bool checkSolidDependencies(const GVolume*                              s,
	                            std::unordered_map<std::string, G4Volume*>* g4s);
	/**
	 * @brief Placeholder logical‑dependency check.
	 *
	 * Currently, returns @c true unconditionally; future extensions may verify
	 * material availability or sensitive  detector prerequisites.
	 *
	 * @note The double [[maybe_unused]] avoids compiler warnings until the body
	 *       is implemented.
	 */
	static bool checkLogicalDependencies(const GVolume*                              s,
	                                     std::unordered_map<std::string, G4Volume*>* g4s);

	/**
	 * @brief Verify that both the *candidate* logical and its *mother* logical
	 *        volumes exist in the map before attempting to build a physical volume.
	 *
	 * @return `true` if the logical prerequisites are satisfied, otherwise `false`.
	 */
	bool checkPhysicalDependencies(const GVolume*                              s,
	                               std::unordered_map<std::string, G4Volume*>* g4s);

	// ──────── geometric helpers ────────────────────────────────────
	static G4RotationMatrix* getRotation(const GVolume* s); ///< Parse rotation string.
	static G4ThreeVector     getPosition(const GVolume* s); ///< Parse position + shift.


	/** Forwarded to `G4PVPlacement` overlap‑check flag. */
	int checkOverlaps{0};

	/**
	 * @brief Backup material (NIST name) used if the requested material is absent.
	 *
	 * Empty string ⇒ no fallback; material lookup failure will become fatal.
	 */
	std::string backupMaterial;

};
