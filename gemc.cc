// c++
#include <iostream>
using namespace std;

// goptions
#include "goptions.h"

// utilities, conventions, options definitions
#include "gemcUtilities.h"
#include "conventions.h"
#include "defineOptions.h"

// gsession
#include "gsession.h"

// geant4
#include "G4UImanager.hh"
#include "G4UIsession.hh"
#include "G4MTRunManager.hh"

int main(int argc, char* argv[])
{
	// the option are loaded in utilities/defineOptions.cc
	// they include the gemc core options and any frameworks options
	// they are parsed from the jcard(s) and command line
	GOptions *gopts = new GOptions(argc, argv, defineOptions());

	// get gui switch
	bool gui = gopts->getSwitch("gui");

	// create QT app if gui is not zero
	createQtApplication(argc, argv, gui);

//	// instantiating new User Interface Messenger
//	// our custom cout destination for the UIM: MasterGeant4.[log, err]
//	G4UImanager* UIM = G4UImanager::GetUIpointer();
//	UIM->SetCoutDestination(new GSession);
//
//	// init geant4 run manager with number of threads coming from options
//	G4MTRunManager *g4MTRunManager = new G4MTRunManager;
//	g4MTRunManager->SetNumberOfThreads(gconf.nthreads);
//


	delete gopts;
	cout << GEMCLOGMSGITEM << " Simulation completed, arrivederci! " << endl << endl;
	return EXIT_SUCCESS;
}



