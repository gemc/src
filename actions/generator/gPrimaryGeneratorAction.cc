// gemc
#include "gparticle_options.h"
#include "gparticle_reader.h"
#include "gPrimaryGeneratorAction.h"

// geant4
#include "G4Event.hh"


// Build the primary-generator action, load the configured particle definitions,
// and guarantee a valid fallback particle when no explicit configuration is present.
GPrimaryGeneratorAction::GPrimaryGeneratorAction(std::shared_ptr<GOptions> gopts) :
	GBase(gopts, GPRIMARYGENERATORACTION_LOGGER),
	gparticleGun(std::make_unique<G4ParticleGun>()) {
	// Inline gparticle definitions are generated for every event. File-backed
	// definitions are event records and are selected by Geant4 event id.
	gparticles           = gparticle::getGParticlesFromOption(gopts, log);
	gparticleFileEvents  = gparticle::getGParticleEventsFromSources(gopts, log);

	if (gparticles.empty() && gparticleFileEvents.empty()) {
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

	const auto event_id = anEvent->GetEventID();
	if (event_id >= 0 && static_cast<size_t>(event_id) < gparticleFileEvents.size()) {
		log->info(2, "Generating gparticlefile event ", event_id,
		          " with ", gparticleFileEvents[static_cast<size_t>(event_id)].size(),
		          " propagated particles");

		for (const auto& gparticle : gparticleFileEvents[static_cast<size_t>(event_id)]) {
			if (gparticle != nullptr) {
				gparticle->shootParticle(gparticleGun.get(), anEvent);
			}
		}
	}
}
