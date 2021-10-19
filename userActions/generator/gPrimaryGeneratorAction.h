#ifndef GPRIMARYGENERATORACTION_H
#define GPRIMARYGENERATORACTION_H 1

// glibrary
#include "gparticleOptions.h"

// geant4
#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4GeneralParticleSource.hh"

class GPrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
public:
	GPrimaryGeneratorAction(GOptions* gopts);
	~GPrimaryGeneratorAction();

public:
	void GeneratePrimaries(G4Event*);

private:
	G4GeneralParticleSource* gparticleGun;

	vector<gparticle::JParticle> jparticles;

};


#endif
