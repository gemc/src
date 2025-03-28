// geant4
#include "G4VisExecutive.hh"
#include "G4RunManagerFactory.hh"
#include "FTFP_BERT.hh"
#include "G4StepLimiterPhysics.hh"

// g4display
#include "g4SceneProperties.h"
#include "g4display.h"
#include "g4display_options.h"

// gemc
#include "glogger.h"

// c++
#include <iostream>
using namespace std;

// qt
#include <QApplication>
#include <QMainWindow>

int main(int argc, char* argv[]) {
	auto gopts = new GOptions(argc, argv, g4display::defineOptions());
	auto log = new GLogger(gopts, G4DISPLAY_LOGGER);
	log->info(0, "g4display", "Starting g4display example...");


	QApplication app(argc, argv);

	G4VisManager* visManager = new G4VisExecutive;
	visManager->Initialize();

	G4SceneProperties* g4SceneProperties = new G4SceneProperties(gopts);

	// main window
	QMainWindow* window = new QMainWindow();
	window->setWindowTitle(QString::fromUtf8("displayUI example"));

	// controls
	G4Display* g4display = new G4Display(gopts, window);
	window->setCentralWidget(g4display);

	if (gopts->getSwitch("gui")) {
		log->info(0, "g4display", "Running in GUI mode...");
		window->show();
		return app.exec();
	}

	log->info(0, "g4display", "Running in command line mode...");

	delete g4display;
	delete window;
	delete g4SceneProperties;
	delete visManager;
	delete log;
	delete gopts;
	return EXIT_SUCCESS;
}
