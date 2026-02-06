#include "gemcUtilities.h"
#include "gemcConventions.h"

// Implementation notes:
// - Doxygen documentation is kept authoritative in gemcUtilities.h.
// - This file only provides brief, non-Doxygen comments to clarify intent and flow.

// geant4 headers
#include "G4Threading.hh"
#include "G4UImanager.hh"

// gemc
#include "glogger.h"
#include "gtouchable.h"

namespace gemc {

// return the number of cores from options.
// if 0 is given, returns max number of available cores
int get_nthreads(const std::shared_ptr<GOptions>& gopts, const std::shared_ptr<GLogger>& log) {
	int useThreads = gopts->getScalarInt("nthreads");

	// Geant4 provides a platform-specific core count helper.
	int ncores = G4Threading::G4GetNumberOfCores();

	// Clamp user request:
	// - 0 means "use all available cores"
	// - values larger than available cores are clamped
	if (useThreads == 0 || useThreads > ncores) useThreads = ncores;

	log->info(0, "Using ", useThreads, " threads out of ", ncores, " available cores.");

	return useThreads;
}

std::vector<std::string> verbosity_commands([[maybe_unused]] const std::shared_ptr<GOptions>& gopts, [[maybe_unused]] const std::shared_ptr<GLogger>& log) {
	std::vector<std::string> cmds;

	// --- Always-quiet commands ---
	// These commands reduce Geant4 output noise for typical production runs.
	cmds.emplace_back("/control/verbose 0");
	cmds.emplace_back("/hit/verbose 0");

	cmds.emplace_back("/process/verbose 0");
	cmds.emplace_back("/process/setVerbose 0 all");
	cmds.emplace_back("/process/had/verbose 0");
	cmds.emplace_back("/process/had/deex/verbose 0");
	cmds.emplace_back("/process/had/cascade 0");
	cmds.emplace_back("/process/em/verbose 0");
	cmds.emplace_back("/process/eLoss/verbose 0");

	cmds.emplace_back("/tracking/verbose 0");
	cmds.emplace_back("/geometry/navigator/verbose 0");

	cmds.emplace_back("/event/verbose 0");
	cmds.emplace_back("/event/stack/verbose 0");

	cmds.emplace_back("/cuts/verbose 0");

	cmds.emplace_back("/run/particle/verbose 0");
	cmds.emplace_back("/run/verbose 0");

	cmds.emplace_back("/material/verbose 0");

	cmds.emplace_back("/vis/verbose 0");
	cmds.emplace_back("/particle/verbose 0");

	// cmds.emplace_back("/control/cout/ignoreInitializationCout 1");
	// cmds.emplace_back("/control/cout/useBuffer 1"); // keep MT output tidy?

	return cmds;
}

std::vector<std::string> initial_commands(const std::shared_ptr<GOptions>& gopts, [[maybe_unused]] const std::shared_ptr<GLogger>& log) {
	// check_overlaps is typically provided by the Geant4 system options set.
	auto check_overlaps = gopts->getScalarInt("check_overlaps"); // notice: from g4system options
	auto gui            = gopts->getSwitch("gui");

	std::vector<std::string> cmds;

	// Batch mode: optionally schedule geometry overlap checks before initialization.
	// Geant4 overlap checks use the current "/geometry/test/..." configuration.
	if (check_overlaps == 2) {
		log->info(0, "Running /geometry/test/run with 50 points.");
		cmds.emplace_back("/geometry/test/resolution 50");
		cmds.emplace_back("/geometry/test/run");
	}
	else if (check_overlaps >= 100) {
		log->info(0, "Running /geometry/test/run with ", check_overlaps, " points.");
		cmds.emplace_back("/geometry/test/resolution " + std::to_string(check_overlaps));
		cmds.emplace_back("/geometry/test/run");
	}

	// A re-initialize is required when:
	// - physics changes
	// - geometry changes
	cmds.emplace_back("/run/initialize");

	// If there is no GUI, initialization commands are sufficient.
	if (!gui) return cmds;

	// GUI mode: set up a minimal visualization scene with trajectories and hits.
	cmds.emplace_back("/vis/drawVolume");

	// Disable auto refresh and quieten vis messages whilst scene and trajectories are established.
	cmds.emplace_back("/vis/viewer/set/autoRefresh false");
	cmds.emplace_back("/vis/viewer/set/viewpointVector -1 0 0");
	cmds.emplace_back("/vis/viewer/set/lightsVector -1 0 0");

	cmds.emplace_back("/vis/scene/add/trajectories smooth");
	cmds.emplace_back("/vis/modeling/trajectories/create/drawByCharge");
	cmds.emplace_back("/vis/modeling/trajectories/drawByCharge-0/default/setDrawStepPts true");
	cmds.emplace_back("/vis/modeling/trajectories/drawByCharge-0/default/setStepPtsSize 2");
	cmds.emplace_back("/vis/scene/add/hits");
	cmds.emplace_back("/vis/scene/endOfEventAction accumulate 10000");

	// Background colors and root volume are the same.
	cmds.push_back("/vis/viewer/set/background 0.46666667 0.53333333 0.6");

	// Re-enable refresh and flush once configuration is complete.
	cmds.emplace_back("/vis/viewer/set/autoRefresh true");
	cmds.emplace_back("/vis/viewer/flush");

	return cmds;
}

// initialize G4MTRunManager
void run_manager_commands([[maybe_unused]] const std::shared_ptr<GOptions>& gopts, const std::shared_ptr<GLogger>& log, const std::vector<std::string>& commands) {
	auto* g4uim = G4UImanager::GetUIpointer();

	// Apply commands sequentially so the UI manager sees the same order as a macro file.
	for (const auto& cmd : commands) {
		log->info(2, "Executing UIManager command: ", cmd);
		g4uim->ApplyCommand(cmd);
	}
}

#include <unistd.h>  // needed for get_pid

void start_random_engine(const std::shared_ptr<GOptions>& gopts, const std::shared_ptr<GLogger>& log) {
	auto randomEngineName = gopts->getScalarString("randomEngine");
	auto seed             = gopts->getScalarInt("seed");

	// If the user did not set a seed, derive one using several fast-changing sources.
	// This helps reduce accidental seed reuse across runs.
	if (seed == SEEDNOTSET) {
		auto timed   = time(NULL);
		auto clockd  = clock();
		auto getpidi = getpid();
		seed         = (G4int)(timed - clockd - getpidi);
	}

	// The names come from the CLHEP library, can be found with
	// grep ": public HepRandomEngine" *.h $CLHEP_BASE_DIR/include/CLHEP/Random/* | awk -Fclass '{print $2}' | awk -F: '{print $1}'
	//
	// Select the engine implementation based on the configured string.
	if (randomEngineName == "DRand48Engine")
		G4Random::setTheEngine(new CLHEP::DRand48Engine(seed));
	else if (randomEngineName == "DualRand")
		G4Random::setTheEngine(new CLHEP::DualRand);
	else if (randomEngineName == "Hurd160Engine")
		G4Random::setTheEngine(new CLHEP::Hurd160Engine);
	else if (randomEngineName == "HepJamesRandom")
		G4Random::setTheEngine(new CLHEP::HepJamesRandom);
	else if (randomEngineName == "MTwistEngine")
		G4Random::setTheEngine(new CLHEP::MTwistEngine);
	else if (randomEngineName == "MixMaxRng")
		G4Random::setTheEngine(new CLHEP::MixMaxRng(seed));
	else if (randomEngineName == "RandEngine")
		G4Random::setTheEngine(new CLHEP::RandEngine);
	else if (randomEngineName == "RanecuEngine")
		G4Random::setTheEngine(new CLHEP::RanecuEngine);
	else if (randomEngineName == "Ranlux64Engine")
		G4Random::setTheEngine(new CLHEP::Ranlux64Engine);
	else if (randomEngineName == "RanluxEngine")
		G4Random::setTheEngine(new CLHEP::RanluxEngine);
	else if (randomEngineName == "RanshiEngine")
		G4Random::setTheEngine(new CLHEP::RanshiEngine);
	else if (randomEngineName == "Hurd288Engine")
		G4Random::setTheEngine(new CLHEP::Hurd288Engine);
	else if (randomEngineName == "TripleRand")
		G4Random::setTheEngine(new CLHEP::TripleRand);
	else { log->error(EC__RANDOMENGINENOTFOUND, "Random engine >", randomEngineName, "< not found. Exiting."); }

	// Apply the seed after selecting the engine so the engine instance is active.
	log->info(0, "Starting random engine ", randomEngineName, " with seed ", seed);
	G4Random::setTheSeed(seed);
}

}
