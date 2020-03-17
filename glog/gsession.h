#ifndef GSESSION_H
#define GSESSION_H 1

// geant4
#include "G4UIsession.hh"

// c++
#include <fstream>
using namespace std;


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
