#include "gemcUtilities.h"
#include "gemcConventions.h"

// Implementation notes:
// - Doxygen documentation is kept authoritative in gemcUtilities.h.
// - This file only provides brief, non-Doxygen comments to clarify intent and flow.

// geant4 headers
#include "G4Threading.hh"
#include "G4UImanager.hh"
#include "G4UnitsTable.hh"

// gemc
#include "glogger.h"
#include "gtouchable.h"
#include "gutilities.h"
#include "g4SceneProperties.h"
#include "g4display_options.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <sstream>

namespace {
bool scalarOptionIsTrue(const std::shared_ptr<GOptions>& gopts, const std::string& name) {
	if (gopts == nullptr) { return false; }

	std::string value = gopts->getScalarString(name);
	std::transform(value.begin(), value.end(), value.begin(),
	               [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

	if (value == "true" || value == "yes" || value == "on" || value == "1") {
		return true;
	}
	if (value == "false" || value == "no" || value == "off" || value == "0" || value == "null") {
		return false;
	}

	std::cerr << FATALERRORL << "The option " << name
	          << " accepts only true/false/yes/no/on/off/1/0." << std::endl;
	std::exit(EC__NOOPTIONFOUND);
}

std::string rootExtentForFieldCommand(const std::shared_ptr<GOptions>& gopts) {
	if (gopts == nullptr) { return ""; }

	std::string rootDefinition = gopts->getScalarString("root");
	for (auto& c : rootDefinition) {
		if (c == ',') { c = ' '; }
	}

	const auto tokens = gutilities::getStringVectorFromString(rootDefinition);
	if (tokens.size() < 5 || tokens[0] != "G4Box") { return ""; }

	const double dx = gutilities::getG4Number(tokens[1]);
	const double dy = gutilities::getG4Number(tokens[2]);
	const double dz = gutilities::getG4Number(tokens[3]);
	if (dx <= 0 || dy <= 0 || dz <= 0) { return ""; }

	std::ostringstream command;
	command << "/vis/set/extentForField "
	        << -dx << " " << dx << " "
	        << -dy << " " << dy << " "
	        << -dz << " " << dz << " mm";
	return command.str();
}
}

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

	std::vector<std::string> verbosity_commands([[maybe_unused]] const std::shared_ptr<GOptions>& gopts,
												[[maybe_unused]] const std::shared_ptr<GLogger>&  log) {
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

	std::vector<std::string> initial_commands(const std::shared_ptr<GOptions>&                 gopts,
											  [[maybe_unused]] const std::shared_ptr<GLogger>& log,
											  bool configure_visualization) {
		// check_overlaps is typically provided by the Geant4 system options set.
		auto check_overlaps = gopts->getScalarInt("check_overlaps"); // notice: from g4system options
		auto gui            = gopts->getSwitch("gui");
		auto g4view = g4display::getG4View(gopts);

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

		// In GUI mode without startup geometry, defer initialization until the setup
		// tab has selected and loaded a real system. Initializing the synthetic ROOT
		// world here leaves stale world/vis state behind after setup-tab reloads.
		if (gui && !configure_visualization) { return cmds; }

		// A re-initialize is required when:
		// - physics changes
		// - geometry changes
		cmds.emplace_back("/run/initialize");

		if (!configure_visualization) return cmds;


		// If there is no GUI, or no need for batch screenshot, initialization commands are enough.
		if (!gui && g4view.driver != "TOOLSSG_OFFSCREEN") return cmds;

		// do not draw volumes in batch screenshot
		if (g4view.driver != "TOOLSSG_OFFSCREEN") {
			auto g4camera = g4display::getG4Camera(gopts);
			auto g4light  = g4display::getG4Light(gopts);
			const double toDegrees = 180.0 / M_PI;
			double thetaValue      = gutilities::getG4Number(g4camera.theta) * toDegrees;
			double phiValue        = gutilities::getG4Number(g4camera.phi)   * toDegrees;
			double lightThetaValue = gutilities::getG4Number(g4light.theta)  * toDegrees;
			double lightPhiValue   = gutilities::getG4Number(g4light.phi)    * toDegrees;
			if (lightThetaValue == 0.0 && lightPhiValue == 0.0) {
				lightThetaValue = thetaValue;
				lightPhiValue   = phiValue;
			}

			cmds.emplace_back("/vis/drawVolume");
			// Disable auto refresh and quieten vis messages whilst scene and trajectories are established.
			cmds.emplace_back("/vis/viewer/set/autoRefresh false");
			cmds.emplace_back("/vis/viewer/set/viewpointThetaPhi " + std::to_string(thetaValue) + " " + std::to_string(phiValue));
			cmds.emplace_back("/vis/viewer/set/lightsThetaPhi "    + std::to_string(lightThetaValue) + " " + std::to_string(lightPhiValue));
		}

		// GUI / batch screenshot mode: set up a minimal visualization scene with trajectories and hits.
		cmds.emplace_back("/vis/scene/add/trajectories smooth");
		cmds.emplace_back("/vis/modeling/trajectories/create/drawByCharge");
		cmds.emplace_back("/vis/modeling/trajectories/drawByCharge-0/default/setDrawStepPts true");
		cmds.emplace_back("/vis/modeling/trajectories/drawByCharge-0/default/setStepPtsSize 2");
		// Draw optical photons in cyan so they are distinct from other neutral particles.
		cmds.emplace_back("/vis/modeling/trajectories/create/drawByParticleID");
		cmds.emplace_back("/vis/modeling/trajectories/drawByParticleID-0/set opticalphoton cyan");

		cmds.emplace_back("/vis/scene/add/hits");
		cmds.emplace_back("/vis/scene/endOfEventAction accumulate 10000");
		cmds.push_back("/vis/viewer/set/background " + g4view.background);
		cmds.push_back("/vis/viewer/set/numberOfCloudPoints " + std::to_string(g4view.cloudPoints));
		G4SceneProperties g4SceneProperties(gopts);
		const auto decorations = g4display::getG4Decorations(gopts);
		for (const auto& command : g4SceneProperties.addSceneDecorations(gopts)) { cmds.emplace_back(command); }
		for (const auto& command : g4SceneProperties.addSceneTexts(gopts)) { cmds.emplace_back(command); }
		if (decorations.eventID) { cmds.emplace_back("/vis/scene/add/eventID"); }

		if (scalarOptionIsTrue(gopts, "show_auxiliary_edges")) {
			cmds.emplace_back("/vis/viewer/set/auxiliaryEdge 1");
			cmds.emplace_back("/vis/viewer/set/hiddenEdge 1");
		}

		const int fieldLinePoints = gopts->getScalarInt("show_field_lines");
		if (fieldLinePoints > 0) {
			if (const auto extent = rootExtentForFieldCommand(gopts); !extent.empty()) {
				cmds.emplace_back(extent);
			}
			cmds.emplace_back("/vis/scene/add/magneticField " + std::to_string(fieldLinePoints));
		}

		// do not draw volumes in batch screenshot
		if (g4view.driver != "TOOLSSG_OFFSCREEN") {
			// Re-enable refresh and flush once configuration is complete.
			cmds.emplace_back("/vis/viewer/set/autoRefresh true");
			cmds.emplace_back("/vis/viewer/flush");
		}

	//	cmds.emplace_back("/tracking/verbose 2");
		return cmds;
	}

	// initialize G4MTRunManager
	void run_manager_commands([[maybe_unused]] const std::shared_ptr<GOptions>& gopts,
							  const std::shared_ptr<GLogger>& log, const std::vector<std::string>& commands) {
		auto* g4uim = G4UImanager::GetUIpointer();

		// Apply commands sequentially so the UI manager sees the same order as a macro file.
		for (const auto& cmd : commands) {
			log->info(2, "Executing UIManager command: ", cmd);
			g4uim->ApplyCommand(cmd);
		}
	}

	void define_new_gemc_units() {
		new G4UnitDefinition("milligray", "milliGy", "Dose", gemc_units::milligray);
		new G4UnitDefinition("microgray", "microGy", "Dose", gemc_units::microgray);
		new G4UnitDefinition("nanogray", "nanoGy", "Dose", gemc_units::nanogray);
		new G4UnitDefinition("picogray", "picoGy", "Dose", gemc_units::picogray);
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
			seed         = (G4int)( timed - clockd - getpidi );
			log->info(1, "Using random seed ", seed);
		} else {
			log->info(1, "User defined seed ", seed);
		}


		// The names come from the CLHEP library, can be found with
		// grep ": public HepRandomEngine" *.h $CLHEP_BASE_DIR/include/CLHEP/Random/* | awk -Fclass '{print $2}' | awk -F: '{print $1}'
		//
		// Select the engine implementation based on the configured string.
		if (randomEngineName == "DRand48Engine")
			G4Random::setTheEngine(new CLHEP::DRand48Engine(seed));
		else if (randomEngineName == "DualRand")
			G4Random::setTheEngine(new CLHEP::DualRand(seed));
		else if (randomEngineName == "Hurd160Engine")
			G4Random::setTheEngine(new CLHEP::Hurd160Engine(seed));
		else if (randomEngineName == "HepJamesRandom")
			G4Random::setTheEngine(new CLHEP::HepJamesRandom(seed));
		else if (randomEngineName == "MTwistEngine")
			G4Random::setTheEngine(new CLHEP::MTwistEngine(seed));
		else if (randomEngineName == "MixMaxRng")
			G4Random::setTheEngine(new CLHEP::MixMaxRng(seed));
		else if (randomEngineName == "RandEngine")
			G4Random::setTheEngine(new CLHEP::RandEngine(seed));
		else if (randomEngineName == "RanecuEngine")
			G4Random::setTheEngine(new CLHEP::RanecuEngine(seed));
		else if (randomEngineName == "Ranlux64Engine")
			G4Random::setTheEngine(new CLHEP::Ranlux64Engine(seed));
		else if (randomEngineName == "RanluxEngine")
			G4Random::setTheEngine(new CLHEP::RanluxEngine(seed));
		else if (randomEngineName == "RanshiEngine")
			G4Random::setTheEngine(new CLHEP::RanshiEngine(seed));
		else if (randomEngineName == "Hurd288Engine")
			G4Random::setTheEngine(new CLHEP::Hurd288Engine(seed));
		else if (randomEngineName == "TripleRand")
			G4Random::setTheEngine(new CLHEP::TripleRand(seed));
		else { log->error(EC__RANDOMENGINENOTFOUND, "Random engine >", randomEngineName, "< not found. Exiting."); }

		// Apply the seed after selecting the engine so the engine instance is active.
		log->info(0, "Starting random engine ", randomEngineName, " with seed ", seed);
		G4Random::setTheSeed(seed);
	}
}
