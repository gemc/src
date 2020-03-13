// c++
#include <iostream>
using namespace std;

// goptions
#include "goptions.h"

// utilities, options definitions
#include "utilities.h"

// geant4
#include "G4UImanager.hh"
#include "G4UIsession.hh"
#include "G4MTRunManager.hh"

int main(int argc, char* argv[])
{
	// the option are loaded in utilities/defineOptions.cc
	// they include the gemc core options and any frameworks options
	GOptions *gopts = new GOptions(argc, argv, defineOptions());
	
	// print settings w/o defaults
	gopts->printSettings(false);
	
	// get gemc configuration options
	auto gconf = goptions::getGConf(gopts);
	
	// create QT app if gui is not zero
	createQtApplication(argc, argv, gconf.gui);
	
	// instantiating new User Interface Messenger
	G4UImanager* UIM = G4UImanager::GetUIpointer();
	
	// cout destination for the UIM
	UIM->SetCoutDestination(new G4UIsession);
	
	
	// init geant4 run manager with number of threads coming from options
	G4MTRunManager *g4MTRunManager = new G4MTRunManager;
	g4MTRunManager->SetNumberOfThreads(gconf.nthreads);
	
	return EXIT_SUCCESS;
}



