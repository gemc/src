#ifndef GSTATEMESSAGE_H
#define GSTATEMESSAGE_H 1

// glibrary
#include "goptions.h"

// geant4
#include "G4UIsession.hh"
using std::atomic;


//Remember:
// Shared classes (cout): geometry and physics tables are shared:
// * G4VUserDetectorConstruction,
// * G4VUserPhysicsList
// * G4VUserActionInitialization
//
// Local thread classes (g4cout):
// * EventManager
// * TrackingManager
// * SteppingManager
// * TransportationManager
// * GeometryManager
// * FieldManager
// * Navigator
// * SensitiveDetectorManager



#define GSTATEMESSAGEHEADER  " »"

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
	GStateMessage(GOptions* gopts, string header, string voptionName) : stateHeader(header) {

		stateCounter = 0;

		verbosity =  gopts->getInt(voptionName);
		if(verbosity >= GVERBOSITY_SUMMARY) {
			G4cout << stateStringHeader()  << "constructor" << G4endl;
		}
	}

	~GStateMessage() {
		if(verbosity >= GVERBOSITY_SUMMARY) {
			G4cout << stateStringHeader() << "destructor" << G4endl;
		}
	}


private:
	string stateHeader;
	int verbosity;

	mutable atomic<int> stateCounter;

	// start of all messages
	string stateStringHeader() const;
	
public:
	void logAlways(const string msg) const;
	void logSummary(const string msg) const;
	void logDetail(const string msg) const;
	void logWarning(const string msg) const;
	void logError(const string msg, const int exitError) const;

};

#endif
