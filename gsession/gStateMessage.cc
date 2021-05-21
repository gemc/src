#include "gStateMessage.h"

// gstring for GWARNING and FATALERRORL
#include "gstring.h"

// start of all messages
string GStateMessage::stateStringHeader() const {
	stateCounter++;
	return string(GFLOWMESSAGEHEADER) + " " + stateName + " [" + to_string(stateCounter) + "] " + string(GFLOWMESSAGEHEADER) + " ";
}

/**
 * @brief Logs independently of the verbosity
 */
void GStateMessage::logAlways(const string msg) const {
		G4cout << stateStringHeader()  << msg << G4endl;
}

/**
 * @brief log summary message, only when verbosity is equal of or greater than GVERBOSITY_SUMMARY
 */
void GStateMessage::logSummary(const string msg) const {
	if(stateVerbosity > GVERBOSITY_SUMMARY) {
		G4cout << stateStringHeader()  << msg << G4endl;
	}
}

/**
 * @brief log debug message, only when verbosity is greater than GVERBOSITY_DETAILS
 */
void GStateMessage::logDetail(const string msg) const {
	if(stateVerbosity > GVERBOSITY_DETAILS) {
		G4cout << stateStringHeader()  << msg << G4endl;
	}
}


/**
 * @brief log warning message
 */
void GStateMessage::logWarning(const string msg) const {
	G4cout << GWARNING << stateStringHeader()  << msg << G4endl;
}

/**
 * @brief log error message
 */
void GStateMessage::logError(const string msg) const {
	G4cout << FATALERRORL << stateStringHeader()  << msg << G4endl;
}

