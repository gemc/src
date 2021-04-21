#ifndef GSTATEMESSAGE_H
#define GSTATEMESSAGE_H 1

// glibrary
#include "goptions.h"

// utilities, options definitions
#include "gemcUtilities.h"

// geant4
#include "G4UIsession.hh"


#define GFLOWMESSAGEHEADER  "♒︎"

/**
 * @class GStateMessage
 * @brief Base class, derived by most gemc objects, to handle log flow.
 * @details This is a base class derived by most gemc objects. It controls the log output using the verbosity.\n
 * The state string is assigned by the class constructor and the verbosity is a simple goptions named \"\<state\>verbosity\". \n
 * The derived classes use three methods to communicate to log:
 * - message: printed independently of the verbosity
 * - summarize: summary message only printed when verbosity is equal or greater GVERBOSITY_SUMMARY
 * - dmessage: debug message only printed when verbosity is equal to GVERBOSITY_DETAILS
 */

class GStateMessage
{
public:
	
	/**
	 * @brief StateMessage constructor. Built using options and the state string
	 * @details The constructor:
	 * - assigns the stateName, printed in the message headers.\n
	 * - finds the state verbosity in the goptions.
	 */
	GStateMessage(GOptions* gopts, string state) : stateName(state) {

		// the verbosity string is the "state" + "verbosity"
		string verbosityString = state + "verbosity";
		stateVerbosity =  gopts->getInt(verbosityString);

		stateCounter = 0;

		if(stateVerbosity >= GVERBOSITY_SUMMARY) {
			G4cout << stateStringHeader()  << "Constructor" << G4endl;
		}
	}

	~GStateMessage() {
		if(stateVerbosity >= GVERBOSITY_SUMMARY) {
			G4cout << stateStringHeader() << "Destructor" << G4endl;
		}
	}


private:
	string stateName;
	int stateVerbosity;

	mutable atomic<int> stateCounter;

	// start of all messages
	string stateStringHeader() const;
	
public:
	void message(const string msg) const;
	void summarize(const string msg) const;
	void dmessage(const string msg) const;
};

#endif
