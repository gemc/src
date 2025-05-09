#pragma once

// glibrary
#include "goptions.h"

// geant4
#include "G4VModularPhysicsList.hh"

// GPhysics Builds and provide a new G4VModularPhysicsList
// rather than being a class derived from G4VModularPhysicsList
class GPhysics {
public:

    GPhysics(GOptions *gopts, std::shared_ptr<GLogger> log);

    ~GPhysics();

    G4VModularPhysicsList *getPhysList() { return physList; }

private:
    // logs physics modules and constructors
    void printAvailable(std::shared_ptr<GLogger> log);

    G4VModularPhysicsList *physList;
};


