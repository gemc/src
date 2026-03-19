// gemc
#include "gparticle_options.h"
#include "gPrimaryGeneratorAction.h"


// Build the primary-generator action, load the configured particle definitions,
// and guarantee a valid fallback particle when no explicit configuration is present.
GPrimaryGeneratorAction::GPrimaryGeneratorAction(std::shared_ptr<GOptions> gopts) :
	GBase(gopts, GPRIMARYGENERATORACTION_LOGGER),
	gparticleGun(std::make_unique<G4ParticleGun>()) {
	// Load all configured particle descriptions from the shared options object.
	gparticles = gparticle::getGParticles(gopts, log);

	if (gparticles.empty()) {
		// Fall back to a default particle definition so the generator remains usable
		// even when no explicit particle configuration was provided.
		auto default_particle = Gparticle::create_default_gparticle(log);
		log->info(1, "No gparticle was defined. Creating default:", *default_particle);
		gparticles.emplace_back(default_particle);
	}
}


// For each configured particle definition, configure the shared particle gun and
// inject the corresponding primary information into the current event.
void GPrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent) {
	for (const auto& gparticle : gparticles) {

		if (gparticle != nullptr) {
			gparticle->shootParticle(gparticleGun.get(), anEvent);
		}
	}
}