// actions
#include "generator/gPrimaryGeneratorAction.h"

// gparticle
#include "gparticle_reader.h"

// geant4
#include "G4Event.hh"
#include "G4RunManagerFactory.hh"
#include "QBBC.hh"

// c++
#include <cstdlib>
#include <iostream>
#include <memory>

int main(int argc, char* argv[]) {
	auto option_definitions = gparticle::defineOptions();
	option_definitions += gprimaryaction::defineOptions();

	auto gopts = std::make_shared<GOptions>(argc, argv, option_definitions);
	auto log   = std::make_shared<GLogger>(gopts, FUNCTION_NAME, GPRIMARYGENERATORACTION_LOGGER);

	auto runManager = G4RunManagerFactory::CreateRunManager(G4RunManagerType::Default);
	runManager->SetUserInitialization(new QBBC);

	auto source_events = gparticle::getGParticleEventsFromSources(gopts, log);
	if (source_events.size() != 11) {
		std::cerr << "Expected 11 gparticlefile events, got " << source_events.size() << '\n';
		delete runManager;
		return EXIT_FAILURE;
	}

	GPrimaryGeneratorAction generator(gopts);
	auto inline_particles = gparticle::getGParticlesFromOption(gopts, log);
	for (size_t event_index = 0; event_index < source_events.size(); event_index++) {
		G4Event event(static_cast<G4int>(event_index));
		generator.GeneratePrimaries(&event);

		const auto expected_vertices = static_cast<G4int>(inline_particles.size() + source_events[event_index].size());
		if (event.GetNumberOfPrimaryVertex() != expected_vertices) {
			std::cerr << "Event " << event_index << " expected " << expected_vertices
			          << " primary vertices, got " << event.GetNumberOfPrimaryVertex() << '\n';
			delete runManager;
			return EXIT_FAILURE;
		}
	}

	delete runManager;
	return EXIT_SUCCESS;
}
