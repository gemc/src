// c++
#include <iostream>

// geant4
#include "G4UImanager.hh"
#include "G4UIsession.hh"
#include "G4RunManagerFactory.hh"
#include "G4VisExecutive.hh"

#include "G4UIterminal.hh"
#include "G4UItcsh.hh"

#include "G4UIQt.hh"
#include "G4TransportationParameters.hh"

// gemc
#include "goptions.h"
#include "gsplash.h"
#include "eventDispenser.h"
#include "g4SceneProperties.h"
#include "gphysics.h"
#include "gemcUtilities.h"
#include "gemcConventions.h"
#include "gemcOptions.h"
#include "gActionInitialization.h"
#include "gui.h"
#include "gbatch_session.h"
#include "gdetectorConstruction.h"
#include "glogger.h"

int main(int argc, char *argv[]) {
	auto gopts = new GOptions(argc, argv, gemc::defineOptions());

	bool gui = gopts->getSwitch("gui");
	bool interactive = gopts->getSwitch("i");
	bool checkForOverlaps = gopts->getSwitch("checkOverlaps");

	GLogger log(gopts, "gemc", "general");

	// splash screen
	auto  gemcSplash = nullptr;

	// createQtApplication returns a QApplication if gui is not zero
	// otherwise it returns a QCoreApplication
	// QScopedPointer os smart pointer in Qt that manages the lifetime of a QCoreApplication
	QScopedPointer <QCoreApplication> gApp(createQtApplication(argc, argv, gui));

	auto UIM = G4UImanager::GetUIpointer();
	if (gui) {
		gemcSplash = new GSplash("gemcArchitecture");
	} else {
		UIM->SetCoutDestination(new GBatch_Session);
	}

	// init geant4 run manager with number of threads coming from options
	auto runManager = G4RunManagerFactory::CreateRunManager(G4RunManagerType::Default);
	runManager->SetNumberOfThreads(getNumberOfThreads(gopts));

	// random engine
	startRandomEngine(gopts);

	// instantiating pointer to global digitization map
	// the map is also used by eventDispenser to reload constants at every run number
	// make this belong to gdetectorconstruction and pass it around from detector construction
	map < string, GDynamicDigitization * > *globalDigitizationMap = new map<string, GDynamicDigitization *>;

	// GDetectorConstruction will run Construct() and load the digitization plugins
	auto gDetectorGlobal = new GDetectorConstruction(gopts, globalDigitizationMap);
	runManager->SetUserInitialization(gDetectorGlobal);

	// starting gphysics
	auto gphysics = new GPhysics(gopts);

	// if showAvailablePhysicsX switch is on, print available physics and exit
	if (gopts->getSwitch("showAvailablePhysicsX")) {
		delete globalDigitizationMap;
		if (gui) {
			delete gemcSplash;
		}
		delete gopts;
		// Free the store: user actions, physics_list and detector_description are
		// owned and deleted by the run manager
		delete runManager;
		return EXIT_SUCCESS;
	}
	runManager->SetUserInitialization(gphysics->getPhysList());

	// instantiate GActionInitialization and initialize the geant4 kernel
	runManager->SetUserInitialization(new GActionInitialization(gopts, globalDigitizationMap));

	G4double warningE = 10.0 * CLHEP::keV;
	G4double importantE = 1 * CLHEP::GeV;
	G4int numTrials = 30;

	auto transportParams = G4TransportationParameters::Instance();
	transportParams->SetWarningEnergy(warningE);
	transportParams->SetImportantEnergy(importantE);
	transportParams->SetNumberOfTrials(numTrials);

	// this initializes g4MTRunManager, which:
	// - calls Construct in GDetectorConstruction (builds gworld and g4world)
	// - calls ConstructSDandField in GDetectorConstruction (builds sensitive detectors)
	initGemcG4RunManager(runManager, gopts);

	// G4VisExecutive can take a verbosity argument - see /vis/verbose guidance
	// notice we initialize this in batch mode as well
	auto visManager = new G4VisExecutive("Quiet");
	visManager->Initialize();

	auto geventDispenser = new EventDispenser(gopts, globalDigitizationMap);



	if (gui) {
		// initializing qt session
		gemcSplash->message("Starting GUI");
		qApp->processEvents();

		// initializing G4UIQt session.
		// Notice g4SceneProperties has to be declared after this, so we have to duplicate it for batch mode
		auto uiQtSession = new G4UIQt(1, argv);
		G4SceneProperties *g4SceneProperties = new G4SceneProperties(gopts);

		// passing executable to retrieve full path
		GemcGUI gemcGui(argv[0], gopts, geventDispenser, gDetectorGlobal);
		gemcGui.show();
		gemcSplash->finish(&gemcGui);

		applyInitialUIManagerCommands(true, checkForOverlaps, verbosity);
		qApp->exec();

		delete g4SceneProperties;
		delete uiQtSession;

	} else {
		auto session = new G4UIterminal(new G4UItcsh);

		// set display properties in batch mode
		auto g4SceneProperties = new G4SceneProperties(gopts);
		applyInitialUIManagerCommands(false, checkForOverlaps, verbosity);

		if (interactive) { session->SessionStart(); }

		geventDispenser->processEvents();
		delete g4SceneProperties;
		delete session;

	}
	// clearing pointers
	// delete visManager; deleting this cause error. Perhaps can define / delete in the functions above
	delete geventDispenser;

	for (auto [key, value]: (*globalDigitizationMap)) { delete value; }
	delete globalDigitizationMap;
	delete gopts;

	// Free the store: user actions, physics_list and detector_description are
	// owned and deleted by the run manager
	delete runManager;

	log.critical("Simulation completed, arrivederci! ");
	return EXIT_SUCCESS;
}
