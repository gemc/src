#pragma once

// gemc
#include "goptions.h"
#include "gparticle_options.h"

// geant4
#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"

constexpr const char* GPRIMARYGENERATORACTION_LOGGER = "GPrimaryGeneratorAction";

namespace gprimaryaction {
inline GOptions defineOptions() { return GOptions(GPRIMARYGENERATORACTION_LOGGER); }
}


class GPrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
public:
	GPrimaryGeneratorAction(std::shared_ptr<GOptions> gopts);
	~GPrimaryGeneratorAction();

public:
	void GeneratePrimaries(G4Event*);

private:

	G4ParticleGun* gparticleGun;

	std::vector<GparticlePtr> gparticles;

	std::shared_ptr<GLogger>  log;

};


