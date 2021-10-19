#include "gPrimaryGeneratorAction.h"


GPrimaryGeneratorAction::GPrimaryGeneratorAction(GOptions* gopts) :
G4VUserPrimaryGeneratorAction(),
gparticleGun(nullptr)
{
	gparticleGun = new G4GeneralParticleSource();
	jparticles = gparticle::getJParticles(gopts);
}

GPrimaryGeneratorAction::~GPrimaryGeneratorAction()
{
	delete gparticleGun;
}



void GPrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{



}
