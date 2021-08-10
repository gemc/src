// c++
#include <iostream>
using namespace std;

// glibrary
#include "goptions.h"
#include "gsplash.h"
#include "eventDispenser.h"
#include "g4display.h"

// geant4
#include "G4UImanager.hh"
#include "G4UIsession.hh"
#include "G4MTRunManager.hh"
#include "G4VisExecutive.hh"
#include "G4UIQt.hh"

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
	gopts->printSettings(true);

	// splash screen
	GSplash  *gemcSplash = nullptr;

	// get gui switch
	bool gui = gopts->getSwitch("gui");
	int verbosity = gopts->getInt("verbosity");

	// createQtApplication returns a QApplication if gui is not zero
	// otherwise it returns a QCoreApplication
	QCoreApplication *gApp = createQtApplication(argc, argv, gui);
	if ( gui ) {
		gemcSplash = new GSplash("gemcArchitecture");
	}

	// instantiating new User Interface Messenger
	// our custom cout destination for the UIM: MasterGeant4.[log, err]
	G4UImanager* UIM = G4UImanager::GetUIpointer();
//	UIM->SetCoutDestination(0);
	UIM->SetCoutDestination(new GSession);

	// init geant4 run manager with number of threads coming from options
	G4MTRunManager *g4MTRunManager = new G4MTRunManager;
	g4MTRunManager->SetNumberOfThreads(getNumberOfThreads(gopts));

	// instantiating pointer to global digitization map
	// the map will be filled with the gsystem information of the sensitive detectors
	map<string, GDynamicDigitization*> *globalDigitizationMap = new map<string, GDynamicDigitization*>;

	// building detector
	// this is global, changed at main scope
	GDetectorConstruction *gDetectorGlobal = new GDetectorConstruction(gopts, globalDigitizationMap);
	g4MTRunManager->SetUserInitialization(gDetectorGlobal);

	// TODO: physics list: to be gphysics
	auto physicsList = new FTFP_BERT;
	g4MTRunManager->SetUserInitialization(physicsList);

	// instantiate GActionInitialization and initialize the geant4 kernel
	g4MTRunManager->SetUserInitialization(new GActionInitialization(gopts, globalDigitizationMap));

	// this Initialize g4MTRunManager, which:
	// calls Construct in GDetectorConstruction
	// calls ConstructSDandField in GDetectorConstruction
	initGemcG4RunManager(g4MTRunManager, gopts);

	EventDispenser *geventDispenser = new EventDispenser(gopts, globalDigitizationMap);


	if ( gui ) {

		gemcSplash->message("Starting GUI");
		qApp->processEvents();

		// passing executable to retrieve full path
		GemcGUI gemcGui(argv[0], gopts, geventDispenser);
		gemcGui.show();
		gemcSplash->finish(&gemcGui);
		delete gemcSplash;


		// initializing vis manager and qt session
		G4VisManager *visManager = new G4VisExecutive();
		visManager->Initialize();

		// intializing G4UIQt session
		G4UIsession *session = new G4UIQt(1, argv);

		// opening the g4Display GUI
		G4Display *g4Display = new G4Display(gopts);

		applyInitialUIManagerCommands(true, verbosity);


		qApp->exec();

		// order of pointers deletion is inverse of creation

		delete g4Display;
		delete session;
		delete visManager;

	} else {
		applyInitialUIManagerCommands(false, verbosity);
		geventDispenser->processEvents();

	}

	// clearing pointers
	delete geventDispenser;
	delete gDetectorGlobal;
	for(auto [key, value]: (*globalDigitizationMap)) { delete value;}
	delete globalDigitizationMap;
	delete UIM;
	delete gApp;
	delete gopts;

	cout << GEMCLOGMSGITEM << "Simulation completed, arrivederci! " << endl << endl;
	return EXIT_SUCCESS;
}



