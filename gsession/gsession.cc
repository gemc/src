// gemc
#include "gsession.h"

// c++
#include <iostream>

using namespace std;

// strategy: if gui is on, logfile will go on a stream to be displayed on the GUI
// in all cases, it will also go to MasterGeant4.*

GSession::GSession() {
    logFile.open("MasterGeant4.log");
    errFile.open("MasterGeant4.err");
}

/**
 * @brief Writes standard output to file MasterGeant4.log
 */
G4int GSession::ReceiveG4cout(const G4String &coutString) {
    logFile << coutString << flush;
	std::cout <<  coutString << std::flush;
    return 0;
}

/**
 * @brief Writes standard error to file MasterGeant4.err
 */
G4int GSession::ReceiveG4cerr(const G4String &cerrString) {
    errFile << cerrString << flush;
    return 0;
}
