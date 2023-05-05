#include "gPrimaryGeneratorAction.h"

// c++
using namespace std;

GPrimaryGeneratorAction::GPrimaryGeneratorAction(GOptions* gopts) :
gparticleGun(nullptr)
{
	gparticleGun = new G4ParticleGun();
	jparticles = gparticle::getJParticles(gopts);
    verbosity = gopts->getInt(GPARTICLEVERBOSITY);

}

GPrimaryGeneratorAction::~GPrimaryGeneratorAction()
{
	delete gparticleGun;
}


void GPrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
	for ( auto& jparticle: jparticles) {
		Gparticle gpar(jparticle);
        if (verbosity >= GVERBOSITY_DETAILS) {
            cout << GPARTICLELOGHEADER << gpar ;
        }
		gpar.shootParticle(gparticleGun, anEvent, verbosity);
	}
}
