#ifndef  GPHYSICS_H
#define  GPHYSICS_H  1

// glibrary
#include "goptions.h"

// geant4
#include "G4VModularPhysicsList.hh"

// GPhysics Builds and provide a new G4VModularPhysicsList
// rather than being a class derived from G4VModularPhysicsList
class  GPhysics 
{
public:
	
	GPhysics(GOptions* gopts);
	~GPhysics();

	G4VModularPhysicsList* getPhysList() { return physList ;}

private:
	// logs physics modules and constructors
	void printAvailable();
	G4VModularPhysicsList* physList;
};


#endif
