#ifndef  GEVENTHEADER_H
#define  GEVENTHEADER_H  1

// gdata
#include "../gdataConventions.h"

// gemc
#include "glogger.h"

// c++
#include "ctime"


class GEventDataCollectionHeader {
public:
	// the event number comes from aEvent->GetEventID(), track id is G4Threading::G4GetThreadId().
	// notice that the logger must come here with the 'gdata' name in the constructor
	GEventDataCollectionHeader(int n, int tid, GLogger *logger) : g4localEventNumber(n), threadID(tid), log(logger) {

		timeStamp = assignTimeStamp();
		log.debug(CONSTRUCTOR, "GEventDataCollectionHeader");
		log.info(1, TPOINTITEM, "Event Number:  ", g4localEventNumber);
		log.info(1, TPOINTITEM, "Thread ID:  ", threadID);
		log.info(1, TPOINTITEM, "Time Stamp:  ", timeStamp);
	}

	~GEventDataCollectionHeader() {
		log.debug(DESTRUCTOR, "GEventDataCollectionHeader");
	}

	inline string const getTimeStamp() const { return timeStamp; }

	inline int getG4LocalEvn() const { return g4localEventNumber; }

	inline int getThreadID() const { return threadID; }


private:
	int g4localEventNumber;  // G4Run-local 
	int threadID;
	GLogger * const log;

	string assignTimeStamp() {
		time_t now = time(NULL);
		struct tm *ptm = localtime(&now);
		char buffer[32];

		// Format: Mo, 15.06.2009 20:20:00
		strftime(buffer, 32, "%a, %m.%d.%Y %H:%M:%S", ptm);

		return string(buffer);
	}

	string timeStamp;
};


#endif
