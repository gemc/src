// geant4
//#include "G4UIsession.hh"
#include "G4RunManagerFactory.hh"
//#include "G4VisExecutive.hh"

#include "G4UIterminal.hh"
#include "G4UItcsh.hh"

//#include "G4UIQt.hh"
//#include "G4TransportationParameters.hh"

// gemc
#include "gemc_options.h"
#include "gemcUtilities.h"
// #include "eventDispenser.h"
// #include "g4SceneProperties.h"
#include "gphysics.h"
// #include "gui.h"
#include "gdetectorConstruction.h"
#include "gaction.h"

int main(int argc, char* argv[]) {
	auto gopts = std::make_shared<GOptions>(argc, argv, gemc::defineOptions());
	auto log   = std::make_shared<GLogger>(gopts, GENERAL_LOGGER, "main");

	auto gui = gopts->getSwitch("gui");
	//	auto interactive = gopts->getSwitch("i");
	auto nthreads = gemc::get_nthreads(gopts, log);

	// createQtApplication returns a QApplication if gui is true
	// otherwise, it returns a QCoreApplication and sets the Geant4 CoutDestination to a GBatch_Session
	auto app = gemc::makeQtApplication(argc, argv, gui);

	// splash screen - nullptr if in batch mode
	auto spash_screen = GSplash::create(gopts, "gemcArchitecture");

	// init geant4 run manager with then number of threads coming from options. always fails if unavailable
	auto runManager = G4RunManagerFactory::CreateRunManager(G4RunManagerType::Default, true, nthreads);

	gemc::start_random_engine(gopts, log);

	auto gdetector = new GDetectorConstruction(gopts);
	runManager->SetUserInitialization(gdetector);

	// starting gphysics
	// exit if phys list requested
	auto gphysics = std::make_shared<GPhysics>(gopts);
	if (gopts->getSwitch("showPhysics")) {
		delete runManager;
		return EXIT_SUCCESS;
	}
	runManager->SetUserInitialization(gphysics->getPhysList());

	log->info(0, "map 1: ", gdetector->get_digitization_routines_map());

	// instantiate GActionInitialization and initialize the geant4 kernel
	runManager->SetUserInitialization(new GAction(gopts, gdetector->get_digitization_routines_map()));


	gemc::init_run_manager(runManager, gopts, log);

	//
	// G4double warningE = 10.0 * CLHEP::keV;
	// G4double importantE = 1 * CLHEP::GeV;
	// G4int numTrials = 30;
	//
	// auto transportParams = G4TransportationParameters::Instance();
	// transportParams->SetWarningEnergy(warningE);
	// transportParams->SetImportantEnergy(importantE);
	// transportParams->SetNumberOfTrials(numTrials);
	//
	// // this initializes g4MTRunManager, which:
	// // - calls Construct in GDetectorConstruction (builds gworld and g4world)
	// // - calls ConstructSDandField in GDetectorConstruction (builds sensitive detectors)
	// initGemcG4RunManager(runManager, gopts);
	//
	// // G4VisExecutive can take a verbosity argument - see /vis/verbose guidance
	// // notice we initialize this in batch mode as well
	// auto visManager = new G4VisExecutive("Quiet");
	// visManager->Initialize();
	//
	// auto geventDispenser = new EventDispenser(gopts, globalDigitizationMap);
	//

	auto app_result = EXIT_SUCCESS;

	if (gui) {
		// 	// initializing qt session
		spash_screen->message("Starting GUI");
		QCoreApplication::processEvents();

		//
		// 	// initializing G4UIQt session.
		// 	// Notice g4SceneProperties has to be declared after this, so we have to duplicate it for batch mode
		// 	auto uiQtSession = new G4UIQt(1, argv);
		// 	G4SceneProperties *g4SceneProperties = new G4SceneProperties(gopts);
		//
		//  passing executable to retrieve full path
		// 	GemcGUI gemcGui(argv[0], gopts, geventDispenser, gDetectorGlobal);
		// 	gemcGui.show();
		// 	gemcSplash->finish(&gemcGui);
		//
		// 	applyInitialUIManagerCommands(true, checkForOverlaps, verbosity);
		// 	qApp->exec();
		//
		app_result = QApplication::exec();

		// 	delete g4SceneProperties;
		// 	delete uiQtSession;
		//
	}
	else {
	//	auto session = new G4UIterminal(new G4UItcsh);
		//
		// 	// set display properties in batch mode
		// 	auto g4SceneProperties = new G4SceneProperties(gopts);
		// 	applyInitialUIManagerCommands(false, checkForOverlaps, verbosity);
		//
		// 	if (interactive) { session->SessionStart(); }
		//
		// 	geventDispenser->processEvents();
		// 	delete g4SceneProperties;
	//	delete session;
	}


	// // clearing pointers
	// // delete visManager; deleting this cause error. Perhaps can define / delete in the functions above
	// delete geventDispenser;

	//
	// Free the store: user actions, physics_list and detector_description are
	// owned and deleted by the run manager
	//delete runManager;

	log->info(0, "Simulation completed, arrivederci! ");

	return app_result;
}
