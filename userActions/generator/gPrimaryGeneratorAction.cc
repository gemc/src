// gemc
#include "gparticleOptions.h"

#include "gPrimaryGeneratorAction.h"

// c++
using namespace std;

GPrimaryGeneratorAction::GPrimaryGeneratorAction(GOptions* gopts) :
gparticleGun(nullptr)
{
	gparticleGun = new G4ParticleGun();
	gparticles = gparticle::getGParticles(gopts);
    verbosity = gopts->getVerbosityFor("gparticle");

}

GPrimaryGeneratorAction::~GPrimaryGeneratorAction()
{
	delete gparticleGun;
}


void GPrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
	for ( auto& gparticle: gparticles) {
        if (verbosity >= GVERBOSITY_DETAILS) {
            cout << GPARTICLELOGHEADER << gparticle ;
        }
        gparticle.shootParticle(gparticleGun, anEvent, verbosity);
	}
}
