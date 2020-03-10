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

int main(int argc, char* argv[])
{
	// the option are loaded in utilities/defineOptions.cc
	// they include the gemc core options and any frameworks options
	GOptions *gopts = new GOptions(argc, argv, defineOptions());

	// print settings w/o defaults
	gopts->printSettings(false);


	bool gui = goptions::getGui(gopts);
	createQtApplication(argc, argv, gui);

	// instantiating new User Interface Messenger
	G4UImanager* UIM = G4UImanager::GetUIpointer();

	// cout destination for the UIM
	// 
	UIM->SetCoutDestination(new G4UIsession);


	return EXIT_SUCCESS;
}



