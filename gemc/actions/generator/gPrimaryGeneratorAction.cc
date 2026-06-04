// gemc
#include "gparticle_options.h"
#include "gparticle_reader.h"
#include "gPrimaryGeneratorAction.h"

// geant4
#include "G4Event.hh"

thread_local GParticleEvent GPrimaryGeneratorAction::current_generated_particles;
thread_local GParticleEvent GPrimaryGeneratorAction::current_generated_tracked_particles;
thread_local GParticleRecordEvent GPrimaryGeneratorAction::current_generated_particle_records;
thread_local GParticleRecordEvent GPrimaryGeneratorAction::current_generated_tracked_particle_records;

namespace {
GParticleRecord make_particle_record(const GparticlePtr& particle) {
	if (particle == nullptr) { return {}; }
	const auto& vertex = particle->getVertex();
	return {
		particle->getName(),
		particle->getPid(),
		particle->getGeneratorType(),
		particle->getMultiplicity(),
		particle->getMomentum(),
		particle->getTheta(),
		particle->getPhi(),
		vertex.x(),
		vertex.y(),
		vertex.z()
	};
}

GParticleRecord make_particle_record(const GparticleRuntimeRecord& particle) {
	return {
		particle.name,
		particle.pid,
		particle.type,
		1,
		particle.p,
		particle.theta,
		particle.phi,
		particle.vertex.x(),
		particle.vertex.y(),
		particle.vertex.z()
	};
}

void append_runtime_records(GParticleRecordEvent& records, const GparticlePtr& particle) {
	if (particle == nullptr) { return; }
	for (const auto& runtime_record : particle->getRuntimeRecords()) {
		records.emplace_back(make_particle_record(runtime_record));
	}
}
}

// Build the primary-generator action, load the configured particle definitions,
// and guarantee a valid fallback particle when no explicit configuration is present.
GPrimaryGeneratorAction::GPrimaryGeneratorAction(std::shared_ptr<GOptions> gopts) :
	GBase(gopts, GPRIMARYGENERATORACTION_LOGGER),
	gparticleGun(std::make_unique<G4ParticleGun>()) {
	// Inline gparticle definitions are generated for every event. File-backed
	// definitions are event records and are selected by Geant4 event id.
	gparticles           = gparticle::getGParticlesFromOption(gopts, log);
	gparticleFileEvents  = gparticle::getGParticleEventsFromSources(gopts, log);
	allGparticleFileRecordEvents = gparticle::getGParticleRecordEventsFromSources(gopts, log);

	if (gparticles.empty() && allGparticleFileRecordEvents.empty()) {
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
	current_generated_particles.clear();
	current_generated_tracked_particles.clear();
	current_generated_particle_records.clear();
	current_generated_tracked_particle_records.clear();

	current_generated_particles.insert(current_generated_particles.end(), gparticles.begin(), gparticles.end());
	current_generated_tracked_particles.insert(current_generated_tracked_particles.end(), gparticles.begin(), gparticles.end());
	current_generated_particle_records.reserve(gparticles.size());
	for (const auto& gparticle : gparticles) {
		current_generated_particle_records.emplace_back(make_particle_record(gparticle));
	}

	const auto event_id = anEvent->GetEventID();
	if (event_id >= 0 && static_cast<size_t>(event_id) < allGparticleFileRecordEvents.size()) {
		const auto& event_particles = allGparticleFileRecordEvents[static_cast<size_t>(event_id)];
		current_generated_particle_records.insert(current_generated_particle_records.end(),
		                                          event_particles.begin(),
		                                          event_particles.end());
	}
	if (event_id >= 0 && static_cast<size_t>(event_id) < gparticleFileEvents.size()) {
		const auto& event_particles = gparticleFileEvents[static_cast<size_t>(event_id)];
		current_generated_tracked_particles.insert(current_generated_tracked_particles.end(),
		                                           event_particles.begin(),
		                                           event_particles.end());
	}

	for (const auto& gparticle : gparticles) {

		if (gparticle != nullptr) {
			gparticle->shootParticle(gparticleGun.get(), anEvent);
			append_runtime_records(current_generated_tracked_particle_records, gparticle);
		}
	}

	if (event_id >= 0 && static_cast<size_t>(event_id) < gparticleFileEvents.size()) {
		log->info(2, "Generating gparticlefile event ", event_id,
		          " with ", gparticleFileEvents[static_cast<size_t>(event_id)].size(),
		          " propagated particles");

		for (const auto& gparticle : gparticleFileEvents[static_cast<size_t>(event_id)]) {
			if (gparticle != nullptr) {
				gparticle->shootParticle(gparticleGun.get(), anEvent);
				append_runtime_records(current_generated_tracked_particle_records, gparticle);
			}
		}
	}
}

const GParticleEvent& GPrimaryGeneratorAction::currentGeneratedParticles() {
	return current_generated_particles;
}

const GParticleEvent& GPrimaryGeneratorAction::currentGeneratedTrackedParticles() {
	return current_generated_tracked_particles;
}

const GParticleRecordEvent& GPrimaryGeneratorAction::currentGeneratedParticleRecords() {
	return current_generated_particle_records;
}

const GParticleRecordEvent& GPrimaryGeneratorAction::currentGeneratedTrackedParticleRecords() {
	return current_generated_tracked_particle_records;
}
