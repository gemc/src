#pragma once

// gemc
#include "goptions.h"
#include "gparticle.h"
#include "gparticleConventions.h"

// geant4
#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"

class GPrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
public:
	GPrimaryGeneratorAction(GOptions* gopts);
	~GPrimaryGeneratorAction();

public:
	void GeneratePrimaries(G4Event*);

private:

    int verbosity;
	G4ParticleGun* gparticleGun;

	vector<Gparticle> gparticles;

};


