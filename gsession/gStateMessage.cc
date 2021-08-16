#include "gStateMessage.h"

// gutilities for GWARNING and FATALERRORL
#include "gutilities.h"

// start of all messages
string GStateMessage::stateStringHeader() const {
	stateCounter++;
	return string(GSTATEMESSAGEHEADER) + " " + stateHeader + " [" + to_string(stateCounter) + "] " + string(GSTATEMESSAGEHEADER) + " ";
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
	if(verbosity >= GVERBOSITY_SUMMARY) {
		G4cout << stateStringHeader()  << msg << G4endl;
	}
}

/**
 * @brief log debug message, only when verbosity is greater than GVERBOSITY_DETAILS
 */
void GStateMessage::logDetail(const string msg) const {
	if(verbosity == GVERBOSITY_DETAILS) {
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
void GStateMessage::logError(const string msg, const int exitError) const {
	G4cerr << FATALERRORL << stateStringHeader()  << msg << G4endl;
	gexit(exitError);
}

