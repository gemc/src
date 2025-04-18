#ifndef GWORLD_H
#define GWORLD_H 1

// gsystem
#include "gsystem.h"
#include "gmodifier.h"
#include "gsystem_options.h"
#include "gsystemFactories/systemFactory.h"

// Standard includes.
#include <map>
#include <string>
#include <vector>

using std::string;
using std::vector;
using std::map;

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
	explicit GWorld(GOptions *gopts);
	GWorld(GOptions *gopts, vector<GSystem> gsystems);

	~GWorld();

	/// Returns the map of GSystem objects.
	[[nodiscard]] map<string, GSystem *> *getSystemsMap() const { return gsystemsMap; }

	/// Returns the number of volumes (systems) in the world.
	[[nodiscard]] int get_number_of_volumes() const { return gsystemsMap->size(); }

	/// Returns a list of sensitive detector names.
	vector<string> getSensitiveDetectorsList();

private:
	GOptions *gopts;
	// Map of system name to GSystem pointers.
	map<string, GSystem *> *gsystemsMap = nullptr;
	// Map of volume name to GModifier pointers.
	map<string, GModifier *> gmodifiersMap = {};

	// Searches for a volume among the systems in gsystemsMap.
	// Used in the constructor to apply modifiers.
	[[nodiscard]] GVolume *searchForVolume(string volumeName, string purpose) const;

	/**
	 * Creates and initializes the system factory map.
	 *
	 * This method internally creates a GManager, registers the required factories,
	 * clears the DL map, and returns a pointer to a map (factory name → GSystemFactory pointer).
	 *
	 * @param gsystemsMap Pointer to the map of GSystem objects.
	 * @param verbosity Verbosity level for logging.
	 * @return Pointer to the created system factory map.
	 */
	map<string, GSystemFactory *> *createSystemFactory(map<string, GSystem *> *gsystemsMap);

	// Create and initialize system factories, and load volume definitions.
	void load_systems();

	// Load GModifiers into gmodifiersMap.
	void load_gmodifiers();

	// assign G4 names for all volumes in every system.
	void assignG4Names();

	std::shared_ptr<GLogger> log;        ///< Logger instance for logging messages.

};

#endif
