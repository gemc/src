#include "gaction.h"
#include <gemc/gfactory/gfactory.h>

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4RunManagerFactory.hh"
#include "G4MTRunManager.hh"
#include "G4SDManager.hh"
#include "G4VSensitiveDetector.hh"
#include "G4VUserDetectorConstruction.hh"
#include "QBBC.hh"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <tuple>
#include <unistd.h>

namespace {
void require(bool condition, const char* message) {
	if (!condition) { throw std::runtime_error(message); }
}

class Sensitive final : public G4VSensitiveDetector {
public:
	Sensitive() : G4VSensitiveDetector("test_sro") { collectionName.insert("hits"); }
	void Initialize(G4HCofThisEvent* collections) override {
		auto* hits = new GHitsCollection("test_sro", "hits");
		collections->AddHitsCollection(GetCollectionID(0), hits);
		const auto* event = G4RunManager::GetRunManager()->GetCurrentEvent();
		if (event->GetEventID() % 4) {
			hits->insert(new GHit(nullptr));
			hits->insert(new GHit(nullptr));
		}
	}
	void EndOfEvent(G4HCofThisEvent*) override {
		auto* manager = G4RunManager::GetRunManager();
		if (manager->GetCurrentRun()->GetRunID() == 1 && manager->GetCurrentEvent()->GetEventID() == 0) {
			const_cast<G4Event*>(manager->GetCurrentEvent())->SetEventAborted();
		}
	}
	G4bool ProcessHits(G4Step*, G4TouchableHistory*) override { return false; }
};

class Detector final : public G4VUserDetectorConstruction {
public:
	explicit Detector(bool no_hits) : empty(no_hits) {}
	G4VPhysicalVolume* Construct() override {
		auto* material = G4NistManager::Instance()->FindOrBuildMaterial("G4_Galactic");
		auto* solid = new G4Box("world", 1 * CLHEP::m, 1 * CLHEP::m, 1 * CLHEP::m);
		auto* logical = new G4LogicalVolume(solid, material, "world");
		return new G4PVPlacement(nullptr, {}, logical, "world", nullptr, false, 0);
	}
	void ConstructSDandField() override {
		if (!empty) { G4SDManager::GetSDMpointer()->AddNewDetector(new Sensitive); }
	}
private:
	bool empty;
};

void check_output(const std::filesystem::path& base, int run, int count, bool serial) {
	std::set<std::size_t> workers;
	for (int crate : {1, 2}) {
		std::ifstream file(base.string() + "_r" + std::to_string(run) + "_c" + std::to_string(crate) + ".txt");
		require(file.is_open(), "Missing crate output file");
		std::vector<std::tuple<int, int, int>> actual;
		std::string tag;
		while (file >> tag && tag == "F") {
			int event, sequence, time;
			std::size_t worker;
			require(static_cast<bool>(file >> event >> sequence >> time >> worker), "Malformed SRO frame");
			actual.emplace_back(event, sequence, time);
			workers.insert(worker);
		}
		std::string reason;
		int safe_time;
		require(tag == "E" && static_cast<bool>(file >> reason >> safe_time), "Missing SRO finalization");
		std::vector<std::tuple<int, int, int>> expected;
		if (run != 1) {
			for (int event = 0; event < count; ++event) {
				if (event % 4) {
					expected.emplace_back(event, crate - 1, event * 10);
					expected.emplace_back(event, crate + 1, event * 10);
				}
			}
		}
		require(actual == expected, "Missing, duplicate, misrouted, or unordered SRO payloads");
		require(reason == (run == 1 ? "interrupted" : "completed"), "Incorrect SRO end reason");
		require(safe_time == (run == 1 ? 0 : count * 10), "Empty/aborted events produced incorrect safe time");
		require(!(file >> tag), "Unexpected records after SRO finalization");
	}
	if (run == 0) { require(serial ? workers.size() == 1 : workers.size() > 1, "Worker coverage missing"); }
}
} // namespace

int main(int argc, char* argv[]) {
	try {
		const auto scratch = std::filesystem::temp_directory_path() / ("gemc-sro-" + std::to_string(getpid()));
		require(std::filesystem::create_directory(scratch), "Could not create unique SRO test directory");
		const auto base = scratch / "output";
		auto definitions = gaction::defineOptions();
		definitions += gstreamer::defineOptions();
		definitions += gparticle::defineOptions();
		definitions.defineOption(GVariable("nthreads", 4, "test workers"), "test workers");
		definitions.defineOption(GVariable("also_reject_true_info", "false", "true information policy"),
		                         "true information policy");
		definitions.defineSwitch("sro_serial", "Exercise sequential Geant4 execution");
		definitions.defineSwitch("sro_empty", "Exercise events without hit collections");
		definitions.defineSwitch("sro_test_worker_failure", "Inject a worker SRO failure");
		std::vector<std::string> args(argv, argv + argc);
		args.push_back("-gparticle=[{name: geantino, p: 1, theta: 0}]");
		args.push_back("-gstreamer=[{format: sro, filename: '" + base.string() +
		               "', implementation: test_sro_implementation}]");
		std::vector<char*> pointers;
		for (auto& argument : args) { pointers.push_back(argument.data()); }
		auto options = std::make_shared<GOptions>(
			static_cast<int>(pointers.size()), pointers.data(), definitions);
		const bool serial = options->getSwitch("sro_serial");
		const bool empty = options->getSwitch("sro_empty");
		require(gstreamer::gstreamersMapPtr(options)->empty(), "SRO leaked into worker streamer maps");
		GManager plugins(options);
		auto digitizers = std::make_shared<gdynamicdigitization::dRoutinesMap>();
		(*digitizers)["test_sro"] = plugins.LoadAndRegisterObjectFromLibrary<GDynamicDigitization>(
			"test_sro_implementation", options);
		auto manager = std::unique_ptr<G4RunManager>(G4RunManagerFactory::CreateRunManager(
			serial ? G4RunManagerType::SerialOnly : G4RunManagerType::MTOnly, true, 4));
		if (auto* mt = dynamic_cast<G4MTRunManager*>(manager.get())) { mt->SetEventModulo(1); }
		manager->SetUserInitialization(new Detector(empty));
		manager->SetUserInitialization(new QBBC);
		manager->SetUserInitialization(new GAction(options, digitizers));
		manager->Initialize();
		const std::vector<int> counts{64, 16, 8};
		for (int run = 0; run < static_cast<int>(counts.size()); ++run) {
			manager->BeamOn(counts[run]);
			if (!empty) { check_output(base, run, counts[run], serial); }
		}
		manager.reset();
		require(std::distance(std::filesystem::directory_iterator(scratch),
		                      std::filesystem::directory_iterator{})
		        == (empty ? 0 : 6), "Expected exactly one output file per crate and run");
		std::filesystem::remove_all(scratch);
		std::cout << "SRO runtime routing, empty events, interruption, and repeated runs passed\n";
		return 0;
	}
	catch (const std::exception& error) {
		std::cerr << error.what() << '\n';
		return 1;
	}
}
