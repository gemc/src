#pragma once

// gsystem
#include "gsystem.h"
#include "gmodifier.h"
#include "gsystem_options.h"
#include "gsystemFactories/systemFactory.h"

// C++
#include <map>
#include <string>
#include <vector>
#include <memory>

/**
 * \ingroup gemc_gsystem_core
 *
 * \class GWorld
 * \brief Builds the “world” as a collection of systems, volumes, and post-load modifiers.
 *
 * GWorld is responsible for the *assembly phase* of detector geometry:
 *
 * - It creates a map of detector systems (GSystem) from configuration (GOptions).
 * - It instantiates the appropriate system factories (derived from GSystemFactory).
 * - It asks each factory to load volumes/materials into each GSystem.
 * - It loads modifiers (GModifier) and applies them to the already-loaded volumes.
 * - It performs a final bookkeeping step assigning fully-qualified Geant4 names to volumes.
 *
 * Ownership model:
 * - The world owns the system map container (gsystemsMap).
 * - Each GSystem owns its internal volume/material containers.
 * - Modifiers are stored separately in gmodifiersMap and applied once during construction.
 *
 * \note This class uses the logger provided by GBase. See module mainpage for verbosity levels.
 */
class GWorld : public GBase<GWorld>
{
public:
	/**
	 * \brief Construct the world from configuration.
	 *
	 * Construction performs the full pipeline:
	 * - load system descriptors via \c gsystem::getSystems();
	 * - create internal system map;
	 * - load systems (factories → volumes/materials);
	 * - load and apply modifiers;
	 * - assign final Geant4 names.
	 *
	 * \param gopts Shared configuration/options instance.
	 */
	explicit GWorld(const std::shared_ptr<GOptions>& gopts);

	/**
	 * \brief Construct the world from an explicit list of pre-built systems.
	 *
	 * This overload is useful when systems are created elsewhere (e.g. by a GUI)
	 * and then passed into the world assembly stage.
	 *
	 * \param gopts Shared configuration/options instance.
	 * \param systems List of systems to adopt into the world map.
	 */
	explicit GWorld(const std::shared_ptr<GOptions>& gopts, SystemList systems);

	/**
	 * \brief Returns a pointer to the internal system map.
	 *
	 * \return Pointer to the internal SystemMap container.
	 * \warning The returned pointer is non-owning; callers must not delete it.
	 */
	[[nodiscard]] SystemMap* getSystemsMap() const { return gsystemsMap.get(); }

	/**
	 * \brief Returns the number of systems currently registered in the world.
	 *
	 * Historically this was referred to as “number of volumes”, but this method
	 * effectively returns the number of top-level systems in \c gsystemsMap.
	 */
	[[nodiscard]] int get_number_of_volumes() const { return static_cast<int>(gsystemsMap->size()); }

	/**
	 * \brief Collect the list of sensitive detector identifiers.
	 *
	 * This walks every system and volume and extracts the digitization string.
	 * A non-empty digitization string indicates that the volume contributes to a
	 * sensitive detector collection.
	 *
	 * \return A de-duplicated list of digitization identifiers.
	 */
	std::vector<std::string> getSensitiveDetectorsList();

private:
	std::shared_ptr<GOptions> gopts; ///< Shared configuration/options used throughout world assembly.

	/// Map of system name → GSystem pointers.
	/// The map object is owned by the world; each system is shared.
	std::unique_ptr<SystemMap> gsystemsMap = std::make_unique<SystemMap>();

	/**
	 * \brief Populate \c gsystemsMap from an input list.
	 *
	 * The method clears the internal map and then inserts each system under a key
	 * derived from its name.
	 *
	 * \param systems List of systems to insert.
	 */
	void create_gsystemsMap(SystemList systems);

	/// Map of volume name → modifier object for that volume.
	/// Modifiers are applied once during construction.
	std::map<std::string, std::shared_ptr<GModifier>> gmodifiersMap;

	/**
	 * \brief Search for a volume among all systems.
	 *
	 * This is used during modifier application and during Geant4-name assignment
	 * to find a mother volume.
	 *
	 * \param volumeName Name of the volume to locate.
	 * \param purpose Human-readable string describing the reason for the lookup (used in logs).
	 * \return Pointer to the found volume.
	 * \note This method logs and terminates (via logger error) if the volume is not found.
	 */
	[[nodiscard]] GVolume* searchForVolume(const std::string& volumeName, const std::string& purpose) const;

	/**
	 * \brief Create and initialize the system factory map.
	 *
	 * This method:
	 * - creates a local GManager;
	 * - registers the required concrete system factories based on system definitions;
	 * - instantiates factories and returns them by value in a map.
	 *
	 * \return Map of factory name → factory instance.
	 */
	std::map<std::string, std::unique_ptr<GSystemFactory>> createSystemFactory();

	/**
	 * \brief Instantiate factories and load volumes/materials for each system.
	 *
	 * Also ensures a world ROOT volume exists (injecting a default one if missing).
	 */
	void load_systems();

	/**
	 * \brief Load modifiers and apply them to their target volumes.
	 *
	 * This reads the modifier list from configuration and performs:
	 * - volume lookup across systems;
	 * - apply shift/tilt/existence toggles on the located volume.
	 */
	void load_gmodifiers();

	/**
	 * \brief Assign Geant4 names for all volumes in every system.
	 *
	 * This final bookkeeping step assigns:
	 * - fully-qualified volume name: \c <system>/<volume>
	 * - fully-qualified mother name: \c <motherSystem>/<motherVolume>
	 *
	 * Special cases:
	 * - ROOT/world and “akasha” mother are treated as top-level.
	 */
	void assignG4Names();
};
