/**
 * \file event_data_example.cpp
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

// gemc
#include "glogger.h"

using namespace std;

// emulation of a run of 10 events
int main(int argc, char *argv[]) {

	// Create GOptions using gdata::defineOptions, which aggregates options from gdata and gtouchable.
	GOptions *gopts = new GOptions(argc, argv, gdata::defineOptions());

	// Create loggers: one for gdata and one for gtouchable.
	GLogger *log = new GLogger(gopts, "gdata");
	GLogger *logt = new GLogger(gopts, "gtouchable");

	int nevents = 10;

	// A run is a collection of events. Here we use a vector to hold pointers to event data collections.
	vector < GEventDataCollection * > *runData = new vector<GEventDataCollection *>;

	for (int evn = 1; evn <= nevents; evn++) {

		// Create an event header for this event.
		// The thread ID is emulated by evn % 3 for demonstration.
		GEventDataCollectionHeader *gheader = new GEventDataCollectionHeader(evn, evn % 3, log);

		// Create the event data collection using the header.
		GEventDataCollection *eventData = new GEventDataCollection(gheader, log);

		// Create a HitBitSet for the hit. In this example, all bits are off ("000000").
		HitBitSet hitBitSet("000000");

		// Create an identity string for the GTouchable.
		string identity = "sector: " + to_string(evn) + ", paddle: " + to_string(evn);
		// Define detector dimensions.
		vector<double> dimensions = {1.0, 20.0, evn * 1.0};

		// Create a GTouchable for the hit.
		GTouchable *ctof = new GTouchable("readout", identity, dimensions, logt);
		// Create a GHit using the GTouchable and the HitBitSet.
		GHit *hit = new GHit(ctof, hitBitSet);

		// Create digitized hit data based on the hit.
		GDigitizedData *thisHitData = new GDigitizedData(hit, log);
		// Include some digitized variables.
		thisHitData->includeVariable(CRATESTRINGID, evn);
		thisHitData->includeVariable(SLOTSTRINGID, 2 * evn);
		thisHitData->includeVariable(CHANNELSTRINGID, 3 * evn);

		// Add the digitized data to the event data collection under the detector name "dc".
		eventData->addDetectorDigitizedData("dc", thisHitData);

		// Add the event data collection to the run.
		runData->push_back(eventData);

		// Cleanup temporary objects (these are managed by the eventData now or no longer needed).
		delete hit;
		delete ctof;
	}

	// we probably want to print events here
	// At this point, one could iterate over runData and print or process events.
	// For demonstration, we'll simply print the event numbers.
	for (size_t i = 0; i < runData->size(); i++) {
		log->info(" > Event ", i + 1, " collected with event number: ", runData->at(i)->getEventNumber());
	}

	for (auto event : *runData) {
		delete event;
	}
	// cleanup
	// deleting log here gives error on linux. should be investigated
	delete runData;
	delete gopts;
	
	return EXIT_SUCCESS;
}
