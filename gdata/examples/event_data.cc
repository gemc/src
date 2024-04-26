// gdata
#include "event/gEventDataCollection.h"

// c++
#include <iostream>
#include <thread>
using std::thread;


// emulation of a run of 10 events
// PRAGMA TODO: fix example, add stream example
//int main(int argc, char* argv[])
int main() {

	int nevents = 10;

	// a run is a collection of 10 events
	vector<GEventDataCollection*> *runData = new vector<GEventDataCollection*>;

	for (int evn = 1; evn <= nevents ; evn++ ) {

		// calling gheader with verbosity 1 for debugging purposes
		// GEventDataCollectionHeader *gheader = new GEventDataCollectionHeader(evn, evn, 1);
		//GEventDataCollection *eventData = new GEventDataCollection(gheader, 1);

		// hit with null G4Step (not used)
		// a bitset
		HitBitSet hitBitSet("000000");
//		GHit *newHit = new GHit(ctof, nullptr, hitBitSet);
//
//		GDigitizedData *thisHitData = new GDigitizedData(newHit);
//		
//		thisHitData->includeVariable(CRATESTRINGID,     evn);
//		thisHitData->includeVariable(SLOTSTRINGID,    2*evn);
//		thisHitData->includeVariable(CHANNELSTRINGID, 3*evn);
//
//		eventData->addDetectorDigitizedData("dc", thisHitData);
//
//		runData->push_back(eventData);
	}

	// we probably want to print events here 

	// now deleting events
	for (auto* edata: (*runData) ) {
		delete edata;
	}

	delete runData;

	return EXIT_SUCCESS;
}
