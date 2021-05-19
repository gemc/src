// c++
#include <iostream>
using namespace std;

// glibrary
#include "goptions.h"
#include "gsplash.h"
#include "gdynamicdigitization.h"

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
	// the gemc goptions are defined in utilities/defineOptions.cc
	// they are merged with the frameworks goptions definitions
	// the goptions are then assigned from the jcard(s) and command line
	GOptions *gopts = new GOptions(argc, argv, gemc::defineOptions());

	// splash screen
	GSplash  *gemcSplash = nullptr;

	// get gui switch
	bool gui = gopts->getSwitch("gui");
	// create QT app if gui is not zero
	QCoreApplication *gApp = createQtApplication(argc, argv, gui);
	if ( gui ) {
		gemcSplash = new GSplash("gemcArchitecture");
	}

	// instantiating new User Interface Messenger
	// our custom cout destination for the UIM: MasterGeant4.[log, err]
	G4UImanager* UIM = G4UImanager::GetUIpointer();
	UIM->SetCoutDestination(new GSession);

	// init geant4 run manager with number of threads coming from options
	G4MTRunManager *g4MTRunManager = new G4MTRunManager;
	g4MTRunManager->SetNumberOfThreads(getNumberOfThreads(gopts));

	// instantiating pointer to global digitization map
	map<string, GDynamicDigitization*> *globalDigitization = new map<string, GDynamicDigitization*>;


	

	gopts->printSettings(true);

	// order of pointers deletion is inverse of creation
	delete globalDigitization;
	delete g4MTRunManager;

	if ( gui ) {
		delete UIM;
		delete gemcSplash;
	}
	delete gApp;
	delete gopts;
	
	cout << GEMCLOGMSGITEM << " Simulation completed, arrivederci! " << endl << endl;
	return EXIT_SUCCESS;
}



