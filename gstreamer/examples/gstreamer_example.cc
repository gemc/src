// gstreamer
#include "gstreamer_options.h"

// gemc
#include "glogger.h"
#include "gdynamicdigitization.h"
#include "gutilities.h"
#include "gdata_options.h"
#include <gtouchable_options.h>


// this example uses the gplugin "test_gdynamic_plugin"
// built by gdynamicDigitization

int main(int argc, char* argv[]) {
	// Create GOptions using gdata::defineOptions, which aggregates options from gdata and gtouchable.
	auto gopts = new GOptions(argc, argv, gstreamer::defineOptions());

	auto log = new GLogger(gopts, GSTREAMER_LOGGER, "gstreamer_example: main");
	auto plugin_log = std::make_shared<GLogger>(gopts, GSTREAMER_LOGGER, "gstreamer_example: plugin manager");
	auto loge = std::make_shared<GLogger>(gopts, DATA_LOGGER, "gstreamer_example example: GEventDataCollection");
	auto logt = std::make_shared<GLogger>(gopts, TOUCHABLE_LOGGER, "gstreamer_example example: GTouchable");

	GManager manager(plugin_log);

	// using dynamicRoutines map
	std::map < std::string, GDynamicDigitization * > dynamicRoutines;
	dynamicRoutines["test"] = manager.LoadAndRegisterObjectFromLibrary<GDynamicDigitization>("test_gdynamic_plugin", gopts);

	log->info(0, "dynamicRoutines[\"test\"]: ", dynamicRoutines["test"]);

	if (dynamicRoutines["test"]->loadConstants(1, "default") == false) {
		log->error(1, "Failed to load constants for dynamic routine 'test' for run number 1 with variation 'default'.");
	}

	// A runData is a collection of events. Here we use a vector to hold pointers to event data collections.
	auto runData = new std::vector<GEventDataCollection*>;

	int nevents = 100;
	int nthreads = 8;
	for (int evn = 1; evn <= nevents; evn++) {
		// Create an event header for this event.
		// The thread ID is emulated by evn % 3 for demonstration.
		auto gheader = new GEventDataCollectionHeader(evn, evn % 3, loge);

		// Create the event data collection using the header.
		auto eventData = new GEventDataCollection(gheader, loge);

		// Create a HitBitSet for the hit. In this example, all bits are off ("000000").
		HitBitSet hitBitSet("000001");

		// Define detector dimensions.
		std::vector<double> dimensions = {1.0, 20.0, evn * 1.0};

		// Create an identity string for the GTouchable.
		std::string identity1 = "sector: " + std::to_string(evn) + ", paddle: " + std::to_string(evn);
		std::string identity2 = "sector: " + std::to_string(evn*2) + ", paddle: " + std::to_string(evn*2);

		// Create a GTouchable for the hit.
		auto ctof1 = new GTouchable("readout", identity1, dimensions, logt);
		auto ctof2 = new GTouchable("readout", identity2, dimensions, logt);


		// Create a GHit using the GTouchable and the HitBitSet.
		auto hit1 = new GHit(ctof1, hitBitSet);
		auto hit2 = new GHit(ctof2, hitBitSet);
		hit1->randomizeHitForTesting(10);
		hit2->randomizeHitForTesting(5);

		auto truedata1 = dynamicRoutines["test"]->collectTrueInformation(hit1,  1);
		auto truedata2 = dynamicRoutines["test"]->collectTrueInformation(hit2,  2);

		auto digidata1 = dynamicRoutines["test"]->digitizeHit(hit1, 1);
		auto digidata2 = dynamicRoutines["test"]->digitizeHit(hit2, 2);

		// Add the digitized data to the event data collection under the detector name "dc".
		eventData->addDetectorDigitizedData("test", digidata1);
		eventData->addDetectorDigitizedData("test", digidata2);

		// // Add the event data collection to the run.
		runData->push_back(eventData);

		// here we write out the true and digitized data


		// Clean up temporary objects (these are managed by the eventData now or no longer needed).
		delete eventData;
		delete hit1;
		delete hit2;
		delete ctof1;
		delete ctof2;
	}





	delete runData;
	delete log;
	delete gopts;
	return EXIT_SUCCESS;
}
