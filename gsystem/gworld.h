#pragma once

// gsystem
#include "gsystem.h"
#include "gmodifier.h"
#include "gsystem_options.h"
#include "gsystemFactories/systemFactory.h"

// Standard includes.
#include <map>
#include <string>
#include <vector>
#include <memory>


/**
 * GWorld is a collection of GSystem objects and their associated modifiers.
 * It loads systems and modifiers, registers system factories, loads volume definitions,
 * and applies modifications.
 */
class GWorld {

public:
	/**
	 * Constructs the world by:
	 *  - Loading systems into gsystemsMap.
	 *  - Loading modifiers into gmodifiersMap.
	 *  - Instantiating and registering system factories.
	 *  - Loading volume definitions for each system.
	 *  - Applying volume modifiers.
	 *
	 * @param gopts Pointer to options.
	 */
	explicit GWorld(GOptions* gopts);
	GWorld(GOptions* g, SystemList systems);

	~GWorld();

	/// Returns the map of GSystem objects.
	[[nodiscard]] SystemMap* getSystemsMap() const { return gsystemsMap.get(); }

	/// Returns the number of volumes (systems) in the world.
	[[nodiscard]] int get_number_of_volumes() const { return static_cast<int>(gsystemsMap->size()); }

	/// Returns a list of sensitive detector names.
	std::vector<std::string> getSensitiveDetectorsList();

private:
	GOptions* gopts;
	// Map of system name to GSystem pointers
	std::unique_ptr<SystemMap> gsystemsMap = std::make_unique<SystemMap>();
	void create_gsystemsMap(SystemList systems);

	// Map of volume name to GModifier pointers.
	std::map<std::string, std::shared_ptr<GModifier>> gmodifiersMap;

	// Searches for a volume among the systems in gsystemsMap.
	// Used in the constructor to apply modifiers.
	[[nodiscard]] GVolume* searchForVolume(const std::string& volumeName, const std::string& purpose) const;

	/**
	 * Creates and initializes the system factory map.
	 *
	 * This method internally creates a GManager, registers the required factories,
	 * clears the DL map, and returns a pointer to a map (factory name → GSystemFactory pointer).
	 *
	 * @param gsystemsMap Pointer to the map of GSystem objects.
	 * @return Pointer to the created system factory map.
	 */
	std::map<std::string, std::unique_ptr<GSystemFactory>> createSystemFactory();



	// Create and initialize system factories, and load volume definitions.
	void load_systems();

	// Load GModifiers into gmodifiersMap.
	void load_gmodifiers();

	// assign G4 names for all volumes in every system.
	void assignG4Names();

	std::shared_ptr<GLogger> log; ///< Logger instance for logging messages.

};
