#pragma once

// gemc
#include "gbase.h"

// geant4
#include "G4VModularPhysicsList.hh"

// GPhysics Builds and provides a new G4VModularPhysicsList
// rather than being a class derived from G4VModularPhysicsList
class GPhysics : public GBase<GPhysics> {
public:
	GPhysics(const std::shared_ptr<GOptions>& gopts);

	[[nodiscard]] G4VModularPhysicsList* getPhysList() const { return physList; }

private:
	// logs physics modules and constructors
	void printAvailable() const;

	G4VModularPhysicsList* physList;
};
