// geant4
#include "G4RunManagerFactory.hh"
#include "G4VisExecutive.hh"
#include "G4UIterminal.hh"
#include "G4UItcsh.hh"
#include "G4UIQt.hh"
#include "G4TransportationParameters.hh"

// gemc
#include "gemc_options.h"
#include "gemcUtilities.h"
#include "eventDispenser.h"
#include "g4SceneProperties.h"
#include "gphysics.h"
#include "gui.h"
#include "gdetectorConstruction.h"
#include "gaction.h"
#include "gstreamer.h"
#include "gsystem_options.h"

// c++
#include <cstdio>
#include <cstdlib>

int main(int argc, char* argv[]) {

	auto base_schema = gemc::defineOptions();
	base_schema += gemc::collectPluginOptions(argc, argv);
	auto gopts = std::make_shared<GOptions>(argc, argv, base_schema);
	auto log   = std::make_shared<GLogger>(gopts, SFUNCTION_NAME, GENERAL_LOGGER);

	auto gui      = gopts->getSwitch("gui");
	auto nthreads = gemc::get_nthreads(gopts, log);
	auto has_startup_geometry = !gsystem::getSystems(gopts).empty();


	// createQtApplication returns a QApplication if gui is true
	// otherwise, it returns a QCoreApplication and sets the Geant4 CoutDestination to a GBatch_Session
	auto app = gemc::makeQtApplication(argc, argv, gui);

	// splash screen - nullptr if in batch mode
	auto spash_screen = GSplash::create(gopts, "gemcArchitecture", 0.7);


	// random engine set by options
	gemc::start_random_engine(gopts, log);

	// init geant4 run manager with then number of threads coming from options. always fails if unavailable
	auto runManager = std::unique_ptr<G4RunManager>(G4RunManagerFactory::CreateRunManager(G4RunManagerType::Default, true, nthreads));

	// Pre-load streamer plugins before Geant4 creates worker threads. Sanitized Linux
	// builds can fail late dlopen() calls from workers with static TLS exhaustion.
	// Declaring this after runManager makes it destruct before runManager at shutdown.
	auto preloaded_gstreamer_map = gstreamer::preloadGStreamerPlugins(gopts);

	// must be a raw pointer because geant4 takes ownership
	auto gdetector = new GDetectorConstruction(gopts);
	runManager->SetUserInitialization(gdetector);

	// define new units
	gemc::define_new_gemc_units();

	// starting gphysics, exit immediately if phys list requested
	auto gphysics = new GPhysics(gopts);
	if (gopts->getSwitch("showPhysics")) { return EXIT_SUCCESS; }
	runManager->SetUserInitialization(gphysics->getPhysList());

	// instantiate GActionInitialization and initialize the geant4 kernel
	runManager->SetUserInitialization(new GAction(gopts, gdetector->get_digitization_routines_map()));

	// sets verbosity commands
	auto verbosities = gemc::verbosity_commands(gopts, log);
	gemc::run_manager_commands(gopts, log, verbosities);



	G4double warningE = 10.0 * CLHEP::keV;
	G4double importantE = 1 * CLHEP::GeV;
	G4int numTrials = 30;

	auto transportParams = G4TransportationParameters::Instance();
	transportParams->SetWarningEnergy(warningE);
	transportParams->SetImportantEnergy(importantE);
	transportParams->SetNumberOfTrials(numTrials);



	// G4VisExecutive can take a verbosity argument - see /vis/verbose guidance
	// notice we initialize this in batch mode as well
	auto visManager = new G4VisExecutive("Quiet");
	visManager->Initialize();

	auto geventDispenser = std::make_shared<EventDispenser>(gopts, gdetector->get_digitization_routines_map());

	auto app_result    = EXIT_SUCCESS;
	auto init_commands = gemc::initial_commands(gopts, log, has_startup_geometry);

	if (gui) {
		// initializing qt session
		spash_screen->message("Starting GUI");
		QCoreApplication::processEvents();

		// initializing G4UIQt session.
		// notice g4SceneProperties has to be declared after this, so we have to duplicate it for batch mode
		auto* uiQtSession       = new G4UIQt(1, argv);
		auto* g4SceneProperties = new G4SceneProperties(gopts);
		std::vector<std::string> scene_commands;
		if (has_startup_geometry) { scene_commands = g4SceneProperties->scene_commands(gopts); }

		// add init_commands to scene_commands
		scene_commands.insert(scene_commands.end(), init_commands.begin(), init_commands.end());

		// run scene_commands
		gemc::run_manager_commands(gopts, log, scene_commands);

		GemcGUI gemcGui(gopts, geventDispenser, gdetector, has_startup_geometry);
		gemcGui.show();

		spash_screen->finish(&gemcGui);
		app_result = QApplication::exec();

		preloaded_gstreamer_map.reset();
		runManager.reset();
		delete g4SceneProperties;
		delete uiQtSession;
	}
	else {
		auto* session           = new G4UIterminal(new G4UItcsh);
		auto* g4SceneProperties = new G4SceneProperties(gopts);
		std::vector<std::string> scene_commands;
		if (has_startup_geometry) { scene_commands = g4SceneProperties->scene_commands(gopts); }

		// set display properties in batch mode
		scene_commands.insert(scene_commands.end(), init_commands.begin(), init_commands.end());
		gemc::run_manager_commands(gopts, log, scene_commands);

		// start the session if interactive
		if (gopts->getSwitch("i")) { session->SessionStart(); }

		geventDispenser->processEvents();

		delete g4SceneProperties;
		delete session;
	}

	geventDispenser.reset();
	preloaded_gstreamer_map.reset();
	runManager.reset();
	// Avoid explicit G4VisExecutive teardown in batch shutdown. On Linux/Geant4 11.4
	// this can double-free the process-wide G4Colour table during static exit.

	log->info(0, "Simulation completed, arrivederci! ");

#if defined(__linux__)
	if (!gui) {
		std::fflush(nullptr);
		std::_Exit(app_result);
	}
#endif

	return app_result;
}
