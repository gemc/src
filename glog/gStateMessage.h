#ifndef GSTATEMESSAGE_H
#define GSTATEMESSAGE_H 1

// glibrary
#include "goptions.h"

// options definitions
#include "defineOptions.h"

// utilities, options definitions
#include "utilities.h"


#define GFLOWMESSAGEHEADER  "♒︎"

// this is a base class derived by most gemc objects
// the class controls the log output and verbosity relative to the derived class
// the state string is assigned by the class constructor
// the state verbosity is a user goption. the option is defined in utilities/defineOptions.cc
// the derived class can send 3 types of messages:
// doutB: debug messages. Only printed if the verbosity is greater or equal GVERBOSITY_ALL
// doutD: details messages. Only printed if the verbosity is greater or equal GVERBOSITY_DETAILS
// dout: normal (summary) messages. Only printed if the verbosity is greater or equal GVERBOSITY_SUMMARY
class GStateMessage
{
public:
	
	GStateMessage(GOptions* gopt, string state) : stateName(state) {

		stateVerbosity = goptions::getVerbosity(gopt, state);

		stateCounter = 0;


		if(stateVerbosity > GVERBOSITY_SILENT) {
			G4cout << stateStringHeader()  << "Constructor" << G4endl;
		}
	}

	~GStateMessage() {
		if(stateVerbosity > GVERBOSITY_SILENT) {
			G4cout << stateStringHeader() << "Destructor" << G4endl;
		}
	}


private:
	string stateName;
	int stateVerbosity;

	mutable atomic<int> stateCounter;

	// start of all messages
	string stateStringHeader() const {
		stateCounter++;
		return string(GFLOWMESSAGEHEADER) + " " + stateName + " [" + to_string(stateCounter) + "] " + string(GFLOWMESSAGEHEADER) + " ";
	}
	
public:
	void stateMessage(const string msg) const {
		if(stateVerbosity > GVERBOSITY_SILENT) {
			G4cout << stateStringHeader()  << msg << G4endl;
		}
	}
	void stateMessage(const vector<string> msgs) const {
		if(stateVerbosity > GVERBOSITY_SILENT) {
			for(auto msg: msgs) {
				G4cout << stateStringHeader()  << msg << G4endl;
			}
		}
	}
};

#endif
