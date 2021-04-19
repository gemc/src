#include "gStateMessage.h"


// start of all messages
string GStateMessage::stateStringHeader() const {
	stateCounter++;
	return string(GFLOWMESSAGEHEADER) + " " + stateName + " [" + to_string(stateCounter) + "] " + string(GFLOWMESSAGEHEADER) + " ";
}

/**
 * @brief Logs independently of the verbosity
 */
void GStateMessage::message(const string msg) const {
		G4cout << stateStringHeader()  << msg << G4endl;
}

/**
 * @brief log summary message, only when verbosity is equal of or greater than GVERBOSITY_SUMMARY
 */
void GStateMessage::summarize(const string msg) const {
	if(stateVerbosity > GVERBOSITY_SUMMARY) {
		G4cout << stateStringHeader()  << msg << G4endl;
	}
}

/**
 * @brief log debug message, only when verbosity is greater than GVERBOSITY_DETAILS
 */
void GStateMessage::dmessage(const string msg) const {
	if(stateVerbosity > GVERBOSITY_DETAILS) {
		G4cout << stateStringHeader()  << msg << G4endl;
	}
}

