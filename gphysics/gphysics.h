#pragma once

// gemc
#include "glogger.h"

// geant4
#include "G4VModularPhysicsList.hh"

// GPhysics Builds and provides a new G4VModularPhysicsList
// rather than being a class derived from G4VModularPhysicsList
class GPhysics {
public:

    GPhysics(GOptions *gopts, std::shared_ptr<GLogger> log);

    ~GPhysics();

    [[nodiscard]] G4VModularPhysicsList *getPhysList() const { return physList; }

private:
    // logs physics modules and constructors
    void printAvailable();
    G4VModularPhysicsList *physList;
	std::shared_ptr<GLogger> log;
};


