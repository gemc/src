/**
 * \file gdata_event_example.cc
 * \brief Example demonstrating event data collection in the GData library.
 *
 * \mainpage GData Event Data Example
 *
 * \section intro_sec Introduction
 * This example emulates a run of 10 events, where each event collects digitized hit data.
 * For each event:
 *  - A GEventDataCollectionHeader is created.
 *  - A GEventDataCollection is instantiated.
 *  - A single hit is created using a GTouchable and a default HitBitSet.
 *  - Digitized data is produced and added to the event's data collection.
 *
 * \section usage_sec Usage
 * Compile and run this example along with the GData library components (GEventDataCollection,
 * GTrueInfoData, GDigitizedData, etc.) and the associated logging and options modules.
 *
 * \n\n
 * \author \n &copy; Maurizio Ungaro
 * \author e-mail: ungaro@jlab.org
 * \n\n\n
 */

// gdata
#include "event/gEventDataCollection.h"
#include "gdata_options.h"
#include "gdataConventions.h"

// gemc
#include "glogger.h"
#include <gtouchable_options.h>

// emulation of a run of 10 events
int main(int argc, char* argv[]) {

	// Create GOptions using gdata::defineOptions, which aggregates options from gdata and gtouchable.
	auto gopts = new GOptions(argc, argv, gdata::defineOptions());

	// Create loggers: one for gdata and one for gtouchable.
	auto log  = std::make_shared<GLogger>(gopts, DATA_LOGGER, "gdata example GEventDataCollection");
	auto logt = std::make_shared<GLogger>(gopts, TOUCHABLE_LOGGER, "gdata example GTouchable");

	int nevents = 10;

	// A run is a collection of events. Here we use a vector to hold pointers to event data collections.
	auto runData = new std::vector<GEventDataCollection*>;

	for (int evn = 1; evn <= nevents; evn++) {
		// Create an event header for this event.
		// The thread ID is emulated by evn % 3 for demonstration.
		auto gheader = new GEventDataCollectionHeader(evn, evn % 3, log);

		// Create the event data collection using the header.
		auto eventData = new GEventDataCollection(gheader, log);

		// Create a HitBitSet for the hit. In this example, all bits are off ("000000").
		HitBitSet hitBitSet("000000");

		// Create an identity string for the GTouchable.
		std::string identity = "sector: " + std::to_string(evn) + ", paddle: " + std::to_string(evn);
		// Define detector dimensions.
		std::vector<double> dimensions = {1.0, 20.0, evn * 1.0};

		// Create a GTouchable for the hit.
		auto ctof = new GTouchable("readout", identity, dimensions, logt);
		// Create a GHit using the GTouchable and the HitBitSet.
		auto hit = new GHit(ctof, hitBitSet);

		// Create digitized hit data based on the hit.
		auto thisHitData = new GDigitizedData(hit, log);
		// Include some digitized variables.
		thisHitData->includeVariable(CRATESTRINGID, evn);
		thisHitData->includeVariable(SLOTSTRINGID, 2 * evn);
		thisHitData->includeVariable(CHANNELSTRINGID, 3 * evn);

		// Add the digitized data to the event data collection under the detector name "dc".
		eventData->addDetectorDigitizedData("dc", thisHitData);

		// Add the event data collection to the run.
		runData->push_back(eventData);

		// Clean up temporary objects (these are managed by the eventData now or no longer needed).
		delete hit;
		delete ctof;
	}

	// For demonstration, we'll simply print the event numbers.
	for (size_t i = 0; i < runData->size(); i++) {
		log->info(" > Event ", i + 1, " collected with event number: ", runData->at(i)->getEventNumber());
	}

	for (auto event : *runData) { delete event; }
	// cleanup
	delete runData;
	delete gopts;

	return EXIT_SUCCESS;
}
