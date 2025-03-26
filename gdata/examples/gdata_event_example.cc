// gdata
#include "event/gEventDataCollection.h"
#include "gdata_options.h"

// gemc
#include "glogger.h"

using namespace std;

// emulation of a run of 10 events
// PRAGMA TODO: stream example
int main(int argc, char *argv[]) {

	GOptions *gopts = new GOptions(argc, argv, gdata::defineOptions());
	GLogger *log = new GLogger(gopts, "gdata");
	GLogger *logt = new GLogger(gopts, "gtouchable");

	int nevents = 10;

	// a run is a collection of 10 events
	vector < GEventDataCollection * > *runData = new vector<GEventDataCollection *>;

	for (int evn = 1; evn <= nevents; evn++) {

		// calling gheader with verbosity 1 for debugging purposes
		GEventDataCollectionHeader *gheader = new GEventDataCollectionHeader(evn, evn % 3, log);
		GEventDataCollection *eventData = new GEventDataCollection(gheader, log);

		// hit with null G4Step (not used)
		// a bitset
		HitBitSet hitBitSet("000000");

		string identity = "sector: " + to_string(evn) + ", paddle: " + to_string(evn);
		vector<double> dimensions = {1.0, 20.0, evn * 1.0};
		GTouchable *ctof = new GTouchable("readout", identity, dimensions, logt);
		GHit *hit = new GHit(ctof, hitBitSet);

		GDigitizedData *thisHitData = new GDigitizedData(hit, log);

		thisHitData->includeVariable(CRATESTRINGID, evn);
		thisHitData->includeVariable(SLOTSTRINGID, 2 * evn);
		thisHitData->includeVariable(CHANNELSTRINGID, 3 * evn);

		eventData->addDetectorDigitizedData("dc", thisHitData);

		runData->push_back(eventData);

		delete thisHitData;
		delete hit;
		delete ctof;
	}

	// we probably want to print events here 

	// cleanup
	delete runData;
	delete gopts;


	return EXIT_SUCCESS;
}
