// gemc
#include "gparticle_options.h"
#include "gPrimaryGeneratorAction.h"


GPrimaryGeneratorAction::GPrimaryGeneratorAction(std::shared_ptr<GOptions> gopts) :
	gparticleGun(nullptr),
	log(std::make_shared<GLogger>(gopts, GPRIMARYGENERATORACTION_LOGGER, "GPrimaryGeneratorAction")) {
 log->debug(CONSTRUCTOR, FUNCTION_NAME);
	gparticleGun = new G4ParticleGun();
	gparticles   = gparticle::getGParticles(gopts, log);
}

GPrimaryGeneratorAction::~GPrimaryGeneratorAction() {
 log->debug(DESTRUCTOR, FUNCTION_NAME);
	delete gparticleGun;
}


void GPrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent) {
	for (auto& gparticle : gparticles) {

		log->info(2, gparticle);

		gparticle->shootParticle(gparticleGun, anEvent, log);
	}
}
