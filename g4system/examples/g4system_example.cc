// gemc
#include "gworld.h"

// g4system
#include "g4world.h"
#include "g4system_options.h"

int main(int argc, char* argv[]) {

    auto gopts =std::make_shared<GOptions>(argc, argv, g4system::defineOptions());
	auto log   = std::make_shared<GLogger>(gopts, SFUNCTION_NAME, G4SYSTEM_LOGGER);

	auto gworld = std::make_shared<GWorld>(gopts);
	auto g4world = std::make_shared<G4World>(gworld.get(), gopts);

	// Print the number of volumes in the gworld
	log->info(2, "Number of volumes in the gworld: ", gworld->get_number_of_volumes());

	// Print the list of sensitive detectors
	auto sensitiveDetectors = gworld->getSensitiveDetectorsList();
	for (const auto& sd : sensitiveDetectors) {
		log->info(2, "Sensitive detector: ", sd);
	}


    return EXIT_SUCCESS;
}
