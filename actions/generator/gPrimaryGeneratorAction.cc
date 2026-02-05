// gemc
#include "gparticle_options.h"
#include "gPrimaryGeneratorAction.h"


GPrimaryGeneratorAction::GPrimaryGeneratorAction(std::shared_ptr<GOptions> gopts) :
	GBase(gopts, GPRIMARYGENERATORACTION_LOGGER),
	gparticleGun(nullptr) {
	// Allocate the particle gun and load configured particle definitions.
	gparticleGun = new G4ParticleGun();
	gparticles   = gparticle::getGParticles(gopts, log);

	if (gparticles.empty()) {
		// Ensure a valid generator configuration by creating a default particle.
		auto default_particle = Gparticle::create_default_gparticle(log);
		log->info(1, "No gparticle was defined. Creating default:", *default_particle);
		gparticles.emplace_back(default_particle);
	}
}

GPrimaryGeneratorAction::~GPrimaryGeneratorAction() {
	// Release the internally owned particle gun.
	delete gparticleGun;
}


void GPrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent) {
	// Generate primaries by iterating over all configured particle definitions.
	for (auto& gparticle : gparticles) {
		log->info(2, gparticle);

		gparticle->shootParticle(gparticleGun, anEvent);
	}
}
