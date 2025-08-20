#pragma once

// gemc
#include "goptions.h"
#include "gbase.h"
#include "gparticle_options.h"

// geant4
#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"

constexpr const char* GPRIMARYGENERATORACTION_LOGGER = "generator";

namespace gprimaryaction {
inline GOptions defineOptions() { return GOptions(GPRIMARYGENERATORACTION_LOGGER); }
}


class GPrimaryGeneratorAction : public GBase<GPrimaryGeneratorAction>, public G4VUserPrimaryGeneratorAction {
public:
	GPrimaryGeneratorAction(std::shared_ptr<GOptions> gopts);
	~GPrimaryGeneratorAction() override;

	void GeneratePrimaries(G4Event*) override;

private:
	G4ParticleGun*            gparticleGun;
	std::vector<GparticlePtr> gparticles;

};
