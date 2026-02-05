#pragma once
/**
 * @file   g4objectsFactory.h
 * @ingroup g4system_geometry
 * @brief  Abstract factory that converts a GEMC DB \c GVolume into Geant4 objects.
 *
 * @details
 * Subclasses implement \ref G4ObjectsFactory::buildSolid "buildSolid()" to create the \c G4VSolid.
 * Default implementations for logical and physical creation are provided and can be overridden when needed.
 *
 * Typical sequence used by the world builder:
 * 1. create or reuse \c G4VSolid
 * 2. create or reuse \c G4LogicalVolume (material + visualization)
 * 3. create or reuse \c G4VPhysicalVolume (placement into mother)
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
 * @ingroup g4system_geometry
 * @brief Base class orchestrating the conversion of a \c GVolume into a Geant4 representation.
 *
 * @details
 * The public entry point \ref G4ObjectsFactory::build_g4volume "build_g4volume()" ensures that a volume has:
 * - a \c G4VSolid
 * - a \c G4LogicalVolume
 * - a \c G4VPhysicalVolume
 *
 * The factory may short-circuit and return \c false when dependencies are not available yet
 * (for example, a mother volume is not built). The world builder can then retry on a later pass.
 */
class G4ObjectsFactory : public GBase<G4ObjectsFactory>
{
public:
	/** Virtual destructor (subclasses may own additional resources). */
	~G4ObjectsFactory() override = default;

	/**
	 * @brief Construct a factory using the global option set.
	 * @param g Shared option set used to initialize the logging base and other behaviors.
	 */
	explicit G4ObjectsFactory(const std::shared_ptr<GOptions>& g) : GBase(g, G4SFACTORY_LOGGER) {
	}

	/**
	 * @brief Configure overlap checking and backup material behavior for this factory.
	 *
	 * @param checkOverlaps  Forwarded to placement calls to enable overlap checking.
	 * @param backupMaterial Material name used if a requested material is missing.
	 *
	 * @details
	 * - If \c backupMaterial is empty, a missing material is treated as an error.
	 * - If overlap checking is enabled, placements may perform overlap checks at construction time.
	 */
	void initialize_context(int                checkOverlaps,
	                        const std::string& backupMaterial);

	/**
	 * @brief Build (or retrieve) solid, logical, and physical volumes for a given \c GVolume.
	 *
	 * @param s   Pointer to the GEMC volume definition.
	 * @param g4s Map that stores and caches the resulting \c G4Volume wrappers.
	 * @return \c true if solid, logical, and physical objects were successfully created; otherwise \c false.
	 *
	 * @details
	 * This method calls:
	 * - \ref G4ObjectsFactory::buildSolid "buildSolid()"
	 * - \ref G4ObjectsFactory::buildLogical "buildLogical()"
	 * - \ref G4ObjectsFactory::buildPhysical "buildPhysical()"
	 *
	 * Implementations are expected to return \c nullptr when dependencies are not satisfied.
	 */
	[[nodiscard]] bool build_g4volume(const GVolume* s,
	                                  std::unordered_map<std::string,
	                                                     G4Volume*>* g4s);

	/**
	 * @brief Short, human-readable factory name for logging.
	 * @return String view valid for the lifetime of the factory.
	 */
	[[nodiscard]] virtual std::string_view className() const = 0;

protected:
	// ──────── pure & virtual hooks for subclasses ──────────────────

	/**
	 * @brief Build the \c G4VSolid for a volume.
	 *
	 * @param s   Volume definition to convert.
	 * @param g4s Map of cached \c G4Volume wrappers.
	 * @return Pointer to the created/retrieved solid, or \c nullptr on failure or missing dependencies.
	 *
	 * @details
	 * Subclasses implement the actual conversion from the GEMC type/parameters to a Geant4 solid.
	 */
	virtual G4VSolid* buildSolid(const GVolume*                              s,
	                             std::unordered_map<std::string, G4Volume*>* g4s) = 0;

	/**
	 * @brief Build or retrieve the \c G4LogicalVolume for a volume.
	 *
	 * @param s   Volume definition to convert.
	 * @param g4s Map of cached \c G4Volume wrappers.
	 * @return Pointer to the created/retrieved logical volume, or \c nullptr if dependencies are missing.
	 *
	 * @details
	 * The default implementation:
	 * - ensures the solid exists (or it was copied from another volume)
	 * - looks up the requested material in the Geant4 material tables
	 * - optionally falls back to the configured backup material
	 * - sets visualization attributes derived from the volume definition
	 */
	virtual G4LogicalVolume* buildLogical(const GVolume*                              s,
	                                      std::unordered_map<std::string, G4Volume*>* g4s);

	/**
	 * @brief Build or retrieve the \c G4VPhysicalVolume for a volume.
	 *
	 * @param s   Volume definition to convert.
	 * @param g4s Map of cached \c G4Volume wrappers.
	 * @return Pointer to the created/retrieved physical volume, or \c nullptr if dependencies are missing.
	 *
	 * @details
	 * The default implementation:
	 * - checks existence and mother/logical dependencies
	 * - computes rotation and translation from the volume definition
	 * - places the logical volume into its mother via \c G4PVPlacement
	 */
	virtual G4VPhysicalVolume* buildPhysical(const GVolume*                              s,
	                                         std::unordered_map<std::string, G4Volume*>* g4s);

	/// Lookup solid in the \c g4s map.
	static G4VSolid* getSolidFromMap(const std::string& volume_name, std::unordered_map<std::string, G4Volume*>* g4s);

	/// Lookup logical volume in the \c g4s map.
	static G4LogicalVolume* getLogicalFromMap(const std::string&                          volume_name,
	                                          std::unordered_map<std::string, G4Volume*>* g4s);

	/// Lookup physical volume in the \c g4s map.
	static G4VPhysicalVolume* getPhysicalFromMap(const std::string&                          volume_name,
	                                             std::unordered_map<std::string, G4Volume*>* g4s);

	/**
	 * @brief Get or create a \c G4Volume wrapper entry in the map.
	 *
	 * @param volume_name Key used for the volume.
	 * @param g4s         Map holding cached wrappers.
	 * @return Pointer to an existing wrapper if found, otherwise a newly allocated wrapper.
	 *
	 * @details
	 * The wrapper is created once and reused for solid/logical/physical caching across build passes.
	 */
	G4Volume* getOrCreateG4Volume(const std::string& volume_name, std::unordered_map<std::string, G4Volume*>* g4s) {
		if (auto it = g4s->find(volume_name); it != g4s->end()) { return it->second; }
		else {
			(*g4s)[volume_name] = new G4Volume();
			return (*g4s)[volume_name];
		}
	}

	/**
	 * @brief Build visualization attributes from the volume definition.
	 * @param s Volume definition containing color/opacity/style/visibility.
	 * @return A \c G4VisAttributes instance configured accordingly.
	 */
	G4VisAttributes createVisualAttributes(const GVolume* s);

	/**
	 * @brief Check whether all prerequisites to build a solid are satisfied.
	 *
	 * @param s   Candidate volume.
	 * @param g4s Map containing all created wrappers so far.
	 * @return \c true if dependencies are satisfied, otherwise \c false.
	 *
	 * @details
	 * The function handles mutually exclusive scenarios:
	 * - "copy of" another volume (solid already exists)
	 * - boolean solid operations (operand solids must exist)
	 * - ordinary primitive solids (no extra prerequisites)
	 */
	bool checkSolidDependencies(const GVolume*                              s,
	                            std::unordered_map<std::string, G4Volume*>* g4s);

	/**
	 * @brief Logical dependency check placeholder.
	 *
	 * @param s   Candidate volume.
	 * @param g4s Map containing all created wrappers so far.
	 * @return Always \c true for now.
	 *
	 * @details
	 * The hook exists for future extensions (material prerequisites, sensitive detector constraints, etc.).
	 */
	static bool checkLogicalDependencies(const GVolume*                              s,
	                                     std::unordered_map<std::string, G4Volume*>* g4s);

	/**
	 * @brief Verify prerequisites to build a physical placement.
	 *
	 * @param s   Candidate volume.
	 * @param g4s Map containing all created wrappers so far.
	 * @return \c true if the candidate logical volume and mother logical volume exist; otherwise \c false.
	 *
	 * @details
	 * This check ensures:
	 * - the candidate wrapper exists in the map
	 * - the candidate logical volume exists (or its copy source logical exists)
	 * - the mother logical exists unless the candidate is the world volume
	 */
	bool checkPhysicalDependencies(const GVolume*                              s,
	                               std::unordered_map<std::string, G4Volume*>* g4s);

	// ──────── geometric helpers ────────────────────────────────────

	/**
	 * @brief Parse rotation string and build a Geant4 rotation matrix.
	 * @param s Volume definition containing rotation specification.
	 * @return Newly allocated \c G4RotationMatrix pointer.
	 */
	static G4RotationMatrix* getRotation(const GVolume* s);

	/**
	 * @brief Parse position and optional shift strings to compute placement translation.
	 * @param s Volume definition containing position and optional shift.
	 * @return Translation vector to use for placement.
	 */
	static G4ThreeVector getPosition(const GVolume* s);

	/** Overlap-check mode forwarded to \c G4PVPlacement. */
	int checkOverlaps{0};

	/**
	 * @brief Backup material name used if the requested material is absent.
	 *
	 * Empty string means: no fallback; missing material lookup becomes fatal.
	 */
	std::string backupMaterial;
};
