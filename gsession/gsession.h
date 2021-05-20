#ifndef GSESSION_H
#define GSESSION_H 1

// geant4
#include "G4UIsession.hh"

// c++
#include <fstream>
using std::ofstream;

/**
 * @class GSession
 * @brief The new G4UIsession passed to the G4UImanager
 * @details An instance of GSession, replacing G4UIsession, is passed to the G4UImanager pointer in gemc.cc main with: SetCoutDestination(new GSession)\n
 * The constructor will create the files for the log and error destinations: MasterGeant4.log and MasterGeant4.err
 */
class GSession : public G4UIsession
{
public:
	GSession();
	G4int ReceiveG4cout(const G4String& coutString);
	G4int ReceiveG4cerr(const G4String& cerrString);

private:
	ofstream logFile;
	ofstream errFile;
};

#endif
