#ifndef  GEVENTHEADER_H
#define  GEVENTHEADER_H  1

// gdata
#include "../gdataConventions.h"

// gemc
#include "glogger.h"

// c++
#include "ctime"



class GEventDataCollectionHeader
{
public:
	// the event number comes from aEvent->GetEventID(), that is why it's called g4EventNumber here
	GEventDataCollectionHeader(int n, int tid, int v = 0) : g4localEventNumber(n), threadID(tid), verbosity(v) {

		timeStamp = assignTimeStamp();

		if ( verbosity >= GVERBOSITY_CLASSES ) {
			gLogClassConstruct("GEventHeader evn " + to_string(g4localEventNumber));
			print();
		}
	}

	~GEventDataCollectionHeader() {
		if ( verbosity >= GVERBOSITY_CLASSES) {
			gLogClassDestruct("GEventHeader evn " + to_string(g4localEventNumber));
		}
	}
	
	inline string const getTimeStamp() const { return timeStamp; }
	inline int getG4LocalEvn()  const  { return g4localEventNumber; }
	inline int getThreadID()    const  { return threadID; }

	void print() {
		cout << CONSTRUCTORLOG <<  " Event header: " << endl;
		cout << TPOINTITEM <<  " Event Number:  "    << g4localEventNumber << endl;
		cout << TPOINTITEM <<  " Thread ID:  "       << threadID  << endl;
		cout << TPOINTITEM <<  " Time Stamp:  "      << timeStamp << endl;
	}

private:
	int g4localEventNumber;  // G4Run-local 
	int threadID;
	int verbosity;

	string assignTimeStamp() {
		time_t now = time(NULL);
		struct tm * ptm = localtime(&now);
		char buffer[32];

		// Format: Mo, 15.06.2009 20:20:00
		strftime (buffer, 32, "%a, %m.%d.%Y %H:%M:%S", ptm);

		return string(buffer);
	}

	string timeStamp;
};


#endif
