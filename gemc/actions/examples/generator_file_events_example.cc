// actions
#include "generator/gPrimaryGeneratorAction.h"

// gparticle
#include "gparticle_reader.h"

// geant4
#include "G4Event.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4RunManagerFactory.hh"
#include "G4Threading.hh"
#include "G4UserEventAction.hh"
#include "G4VUserActionInitialization.hh"
#include "G4VUserDetectorConstruction.hh"
#include "QBBC.hh"

// c++
#include <atomic>
#include <cstdlib>
#include <iostream>
#include <memory>

namespace {
constexpr int stress_events = 512;
constexpr int stress_multiplicity = 32;

struct StressResults {
	std::atomic<int> checked_events{0};
	std::atomic<int> invalid_events{0};
	std::atomic<unsigned int> worker_mask{0};
};

class EmptyDetectorConstruction final : public G4VUserDetectorConstruction {
public:
	G4VPhysicalVolume* Construct() override {
		auto* material = G4NistManager::Instance()->FindOrBuildMaterial("G4_Galactic");
		auto* solid = new G4Box("generator_stress_world", 1 * CLHEP::m, 1 * CLHEP::m, 1 * CLHEP::m);
		auto* logical = new G4LogicalVolume(solid, material, "generator_stress_world");
		return new G4PVPlacement(nullptr, {}, logical, "generator_stress_world", nullptr, false, 0);
	}
};

class RuntimeRecordCheckingAction final : public G4UserEventAction {
public:
	explicit RuntimeRecordCheckingAction(std::shared_ptr<StressResults> results) : results_(std::move(results)) {}

	void EndOfEventAction(const G4Event* event) override {
		const auto worker_id = G4Threading::G4GetThreadId();
		if (worker_id >= 0 && worker_id < 4) {
			results_->worker_mask.fetch_or(1U << worker_id, std::memory_order_relaxed);
		}
		const auto& records = GPrimaryGeneratorAction::currentGeneratedTrackedParticleRecords();
		if (records.size() != stress_multiplicity ||
		    event->GetNumberOfPrimaryVertex() != stress_multiplicity) {
			results_->invalid_events.fetch_add(1, std::memory_order_relaxed);
		}
		results_->checked_events.fetch_add(1, std::memory_order_relaxed);
	}

private:
	std::shared_ptr<StressResults> results_;
};

class SharedGeneratorActionInitialization final : public G4VUserActionInitialization {
public:
	SharedGeneratorActionInitialization(std::shared_ptr<GOptions> options,
	                                    std::shared_ptr<std::vector<GparticlePtr>> particles,
	                                    std::shared_ptr<StressResults> results) :
		options_(std::move(options)), particles_(std::move(particles)), results_(std::move(results)) {}

	void Build() const override {
		SetUserAction(new GPrimaryGeneratorAction(options_, particles_));
		SetUserAction(new RuntimeRecordCheckingAction(results_));
	}

private:
	std::shared_ptr<GOptions> options_;
	std::shared_ptr<std::vector<GparticlePtr>> particles_;
	std::shared_ptr<StressResults> results_;
};
}

int main(int argc, char* argv[]) {
	auto option_definitions = gparticle::defineOptions();
	option_definitions += gprimaryaction::defineOptions();

	auto gopts = std::make_shared<GOptions>(argc, argv, option_definitions);
	auto log   = std::make_shared<GLogger>(gopts, FUNCTION_NAME, GPRIMARYGENERATORACTION_LOGGER);

	auto runManager = G4RunManagerFactory::CreateRunManager(G4RunManagerType::Default);
	runManager->SetNumberOfThreads(4);
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

		const auto expected_vertices = static_cast<G4int>(
			inline_particles.size() + source_events[event_index].size());
		if (event.GetNumberOfPrimaryVertex() != expected_vertices) {
			std::cerr << "Event " << event_index << " expected " << expected_vertices
			          << " primary vertices, got " << event.GetNumberOfPrimaryVertex() << '\n';
			delete runManager;
			return EXIT_FAILURE;
		}
		const auto& tracked_records = GPrimaryGeneratorAction::currentGeneratedTrackedParticleRecords();
		if (tracked_records.size() != static_cast<size_t>(expected_vertices)) {
			std::cerr << "Event " << event_index << " expected " << expected_vertices
			          << " tracked runtime records, got " << tracked_records.size() << '\n';
			delete runManager;
			return EXIT_FAILURE;
		}
	}

	// Reproduce the GUI ownership model: every worker receives the same particle-definition vector. Runtime
	// records must remain worker-local even when randomized multiplicity creates substantial overlap.
	char* stress_argv[] = {argv[0], nullptr};
	auto stress_definitions = gparticle::defineOptions();
	stress_definitions += gprimaryaction::defineOptions();
	auto stress_options = std::make_shared<GOptions>(1, stress_argv, stress_definitions);
	auto stress_log = std::make_shared<GLogger>(stress_options, FUNCTION_NAME, GPRIMARYGENERATORACTION_LOGGER);
	auto stress_particles = std::make_shared<std::vector<GparticlePtr>>();
	stress_particles->emplace_back(std::make_shared<Gparticle>(
		"geantino", stress_multiplicity, 1000 * CLHEP::MeV, 0, "uniform",
		90 * CLHEP::deg, 0, "uniform", 0, 360 * CLHEP::deg,
		0, 0, 0, 0, 0, 0, "uniform", stress_log));
	auto stress_results = std::make_shared<StressResults>();

	runManager->SetUserInitialization(new EmptyDetectorConstruction());
	runManager->SetUserInitialization(
		new SharedGeneratorActionInitialization(stress_options, stress_particles, stress_results));
	runManager->Initialize();
	runManager->BeamOn(stress_events);

	constexpr unsigned int expected_worker_mask = (1U << 4U) - 1U;
	if (stress_results->checked_events.load(std::memory_order_relaxed) != stress_events ||
	    stress_results->invalid_events.load(std::memory_order_relaxed) != 0 ||
	    stress_results->worker_mask.load(std::memory_order_relaxed) != expected_worker_mask) {
		std::cerr << "Shared-generator stress test checked "
		          << stress_results->checked_events.load(std::memory_order_relaxed) << " of " << stress_events
		          << " events and found " << stress_results->invalid_events.load(std::memory_order_relaxed)
		          << " invalid worker-local record sets; worker mask: "
		          << stress_results->worker_mask.load(std::memory_order_relaxed) << '\n';
		delete runManager;
		return EXIT_FAILURE;
	}

	delete runManager;
	return EXIT_SUCCESS;
}
