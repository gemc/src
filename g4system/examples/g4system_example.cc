// gemc
#include "gworld.h"

// g4system
#include "g4world.h"
#include "g4system_options.h"

int main(int argc, char* argv[]) {

    auto gopts = new GOptions(argc, argv, g4system::defineOptions());
	auto log   = std::make_shared<GLogger>(gopts, G4SYSTEM_LOGGER, "g4system_example");

	auto gworld = new GWorld(gopts);
	auto g4world = new G4World(gworld, gopts);

	// Print the number of volumes in the gworld
	log->info(2, "Number of volumes in the gworld: ", gworld->get_number_of_volumes());

	// Print the list of sensitive detectors
	auto sensitiveDetectors = gworld->getSensitiveDetectorsList();
	for (const auto& sd : sensitiveDetectors) {
		log->info(2, "Sensitive detector: ", sd);
	}

	// Clean up
	delete g4world;
	delete gworld;
	delete gopts;

    return EXIT_SUCCESS;
}
