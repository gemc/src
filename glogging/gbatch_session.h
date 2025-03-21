#ifndef GSESSION_H
#define GSESSION_H 1

// geant4
#include "G4UIsession.hh"

// c++
#include <fstream>

/**
 * @class GBatch_Session
 * @brief The new G4UIsession passed to the G4UImanager
 * @details An instance of GBatch_Session, replacing G4UIsession, is passed to the G4UImanager pointer in gemc.cc main
 * to set the G4out Destination\n
 * The constructor will create the files for the log and error destinations: gemc.log and gemc.err
 */
class GBatch_Session : public G4UIsession {
public:
	GBatch_Session() {
		logFile.open("gemc.log");
		errFile.open("gemc.err");
	}

	G4int ReceiveG4cout(const G4String &coutString) {
		logFile << coutString << flush;
		cout << coutString << flush;
		return 0;
	}

	G4int ReceiveG4cerr(const G4String &cerrString) {
		errFile << cerrString << flush;
		cerr << cerrString << flush;
		return 0;
	}

private:
	std::ofstream logFile;
	std::ofstream errFile;
};

#endif
