#include "gPrimaryGeneratorAction.h"


GPrimaryGeneratorAction::GPrimaryGeneratorAction(GOptions* gopts) :
gparticleGun(nullptr)
{
	gparticleGun = new G4ParticleGun();
	jparticles = gparticle::getJParticles(gopts);
}

GPrimaryGeneratorAction::~GPrimaryGeneratorAction()
{
	delete gparticleGun;
}



void GPrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
	for ( auto& jparticle: jparticles) {
		Gparticle gpar(jparticle);
		gpar.shootParticle(gparticleGun, anEvent);
	}
}
