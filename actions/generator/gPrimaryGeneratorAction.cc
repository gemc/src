// gemc
#include "gparticle_options.h"
#include "gPrimaryGeneratorAction.h"


GPrimaryGeneratorAction::GPrimaryGeneratorAction(std::shared_ptr<GOptions> gopts) :
	GBase(gopts, GPRIMARYGENERATORACTION_LOGGER),
	gparticleGun(nullptr) {
	gparticleGun = new G4ParticleGun();
	gparticles   = gparticle::getGParticles(gopts, log);

	if (gparticles.empty()) {
		auto default_particle = Gparticle::create_default_gparticle(log);
		log->info(1, "No gparticle was defined. Creating default:", *default_particle);
		gparticles.emplace_back(default_particle);
	}
}

GPrimaryGeneratorAction::~GPrimaryGeneratorAction() { delete gparticleGun; }


void GPrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent) {
	for (auto& gparticle : gparticles) {
		log->info(2, gparticle);

		gparticle->shootParticle(gparticleGun, anEvent);
	}
}
