// c++
#include <iostream>
using namespace std;

// glibrary
#include "goptions.h"
#include "gsplash.h"
#include "eventDispenser.h"
#include "g4SceneProperties.h"

// geant4
#include "G4UImanager.hh"
#include "G4UIsession.hh"
#include "G4RunManagerFactory.hh"
#include "G4VisExecutive.hh"
#include "G4UIQt.hh"

#include "G4StepLimiterPhysics.hh"

// gemc
#include "gemcUtilities.h"
#include "gemcConventions.h"
#include "gemcOptions.h"
#include "gActionInitialization.h"
#include "gui.h"
#include "gsession.h"
#include "gdetectorConstruction.h"



// TODO: physics list: to be gphysics
#include "FTFP_BERT.hh"


int main(int argc, char* argv[])
{
	// the gemc goptions are defined in utilities/defineOptions.cc
	// they are merged with the frameworks goptions definitions
	// the goptions are then assigned from the jcard(s) and command line
	GOptions *gopts = new GOptions(argc, argv, gemc::defineOptions());
	gopts->printSettings(gopts->getSwitch("sndf"));

	// splash screen
	GSplash  *gemcSplash = nullptr;

	// get gui switch, overlaps check and verbosity
	bool gui             = gopts->getSwitch("gui");
	int checkForOverlaps = gopts->getInt("checkOverlaps");
	int verbosity        = gopts->getInt("verbosity");

	// createQtApplication returns a QApplication if gui is not zero
	// otherwise it returns a QCoreApplication
	QCoreApplication *gApp = createQtApplication(argc, argv, gui);
	if ( gui ) {
		gemcSplash = new GSplash("gemcArchitecture");
	}

	// instantiating new User Interface Messenger
	// our custom cout destination for the UIM: MasterGeant4.[log, err]
	G4UImanager* UIM = G4UImanager::GetUIpointer();
	UIM->SetCoutDestination(new GSession);

	// init geant4 run manager with number of threads coming from options
	auto* runManager =     G4RunManagerFactory::CreateRunManager(G4RunManagerType::Default);
	runManager->SetNumberOfThreads(getNumberOfThreads(gopts));

	// instantiating pointer to global digitization map
	// the map will be filled with the gsystem information of the sensitive detectors
	map<string, GDynamicDigitization*> *globalDigitizationMap = new map<string, GDynamicDigitization*>;

	// building detector
	// this is global, changed at main scope
	GDetectorConstruction *gDetectorGlobal = new GDetectorConstruction(gopts, globalDigitizationMap);
	runManager->SetUserInitialization(gDetectorGlobal);

	// TODO: physics list: to be gphysics
	auto physicsList = new FTFP_BERT();
	physicsList->RegisterPhysics(new G4StepLimiterPhysics());
	runManager->SetUserInitialization(physicsList);

	// instantiate GActionInitialization and initialize the geant4 kernel
	runManager->SetUserInitialization(new GActionInitialization(gopts, globalDigitizationMap));

	// this Initialize g4MTRunManager, which:
	// calls Construct in GDetectorConstruction
	// calls ConstructSDandField in GDetectorConstruction
	// which in turns builds the gsystems, the g4systems, and the sensitive detectors in each thread,
	initGemcG4RunManager(runManager, gopts);

	loadDigitizationPlugins(gopts, gDetectorGlobal->getDigitizationNamesList(), globalDigitizationMap);

	EventDispenser *geventDispenser = new EventDispenser(gopts, globalDigitizationMap);

	// G4VisExecutive can take a verbosity argument - see /vis/verbose guidance
	// notice we initialize this in batch mode as well
	G4VisManager *visManager = new G4VisExecutive("Quiet");
//	G4VisManager* visManager = new G4VisExecutive;
	visManager->Initialize();

	if ( gui ) {

		// initializing qt session
		gemcSplash->message("Starting GUI");
		qApp->processEvents();

		// passing executable to retrieve full path
		GemcGUI gemcGui(argv[0], gopts, geventDispenser);
		gemcGui.show();
		gemcSplash->finish(&gemcGui);

		// intializing G4UIQt session
		G4UIsession *session = new G4UIQt(1, argv);

		// set display properties
		G4SceneProperties *g4SceneProperties = new G4SceneProperties(gopts);

		applyInitialUIManagerCommands(true, checkForOverlaps, verbosity);

		qApp->exec();

		// order of pointers deletion is inverse of creation
		delete g4SceneProperties;
		delete session;

	} else {
		// set display properties in batch mode
		G4SceneProperties *g4SceneProperties = new G4SceneProperties(gopts);

		applyInitialUIManagerCommands(false, checkForOverlaps, verbosity);
		geventDispenser->processEvents();

		delete g4SceneProperties;
	}

	// clearing pointers
	// delete visManager; deleting this cause error. Perhaps can define / delete in the functions above
	delete geventDispenser;

	for(auto [key, value]: (*globalDigitizationMap)) { delete value;}
	delete globalDigitizationMap;

	delete gApp;
	delete gopts;

	// Free the store: user actions, physics_list and detector_description are
	// owned and deleted by the run manager
	delete runManager;

	cout << GEMCLOGMSGITEM << KGRN << "Simulation completed, arrivederci! " << RST << endl << endl;
	return EXIT_SUCCESS;
}

