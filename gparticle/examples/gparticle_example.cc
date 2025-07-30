// gparticle
#include "gparticle_options.h"

// geant4
#include "G4RunManagerFactory.hh"
#include "QBBC.hh"


int main(int argc, char* argv[]) {
	auto gopts =std::make_shared<GOptions>(argc, argv, gparticle::defineOptions());
	auto log   = std::make_shared<GLogger>(gopts, GPARTICLE_LOGGER, "gparticle example: main");

	auto runManager = G4RunManagerFactory::CreateRunManager(G4RunManagerType::Default);

	auto physicsList = new QBBC;
	runManager->SetUserInitialization(physicsList);
	auto particles = gparticle::getGParticles(gopts, log);

	delete runManager;

	return EXIT_SUCCESS;
}
