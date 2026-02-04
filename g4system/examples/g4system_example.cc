/**
* @file   g4system_example.cc
 * @ingroup Geometry
 *
 * @anchor g4system_example_anchor
 *
 * @brief Example showing how to instantiate g4system options, build a GEMC world, and build the Geant4 world.
 *
 * @details
 * This example demonstrates:
 * - registering g4system options via \ref g4system::defineOptions "defineOptions()"
 * - building a GEMC world (GWorld)
 * - building the Geant4 world (G4World)
 * - retrieving basic diagnostics (volume count and sensitive detector list)
 *
 * The example intentionally keeps the logic minimal so it can be used as a template.
 */

// gemc
#include "gworld.h"

// g4system
#include "g4world.h"
#include "g4system_options.h"

int main(int argc, char* argv[]) {
	// Create global option set with g4system options registered.
	auto gopts = std::make_shared<GOptions>(argc, argv, g4system::defineOptions());

	// Logger used by this example. Verbosity is controlled by the option set.
	auto log = std::make_shared<GLogger>(gopts, SFUNCTION_NAME, G4SYSTEM_LOGGER);

	// Build the GEMC world definition from the configured options/sources.
	auto gworld = std::make_shared<GWorld>(gopts);

	// Convert the GEMC world into Geant4 volumes using the g4system module.
	auto g4world = std::make_shared<G4World>(gworld.get(), gopts);

	// Print the number of volumes in the GEMC world definition.
	log->info(2, "Number of volumes in the gworld: ", gworld->get_number_of_volumes());

	// Print the list of sensitive detectors declared by the world definition.
	auto sensitiveDetectors = gworld->getSensitiveDetectorsList();
	for (const auto& sd : sensitiveDetectors) {
		log->info(2, "Sensitive detector: ", sd);
	}

	return EXIT_SUCCESS;
}
