// geant4
#include "G4VisExecutive.hh"
#include "G4RunManagerFactory.hh"
#include "FTFP_BERT.hh"
#include "G4StepLimiterPhysics.hh"

// g4dialog
#include "g4dialog.h"
#include "g4dialog_options.h"
#include "gui_session.h"

// c++
#include <iostream>
using namespace std;

// qt
#include <QApplication>
#include <QMainWindow>

int main(int argc, char* argv[]) {

	auto gopts = new GOptions(argc, argv, g4dialog::defineOptions());
	auto log = new GLogger(gopts, G4DIALOG_LOGGER);

	QApplication app(argc, argv);

	auto visManager = new G4VisExecutive;
	visManager->Initialize();

	// main window
	auto window = new QMainWindow();
	window->setWindowTitle(QString::fromUtf8("displayUI example"));

	// controls
	auto g4dialog = new G4Dialog(gopts, window);
	window->setCentralWidget(g4dialog);


	log->info(0, "g4 dialog : GUI_Session created");

	if (gopts->getSwitch("gui")) {
		window->show();
		return app.exec();
	}

	delete g4dialog;
	delete window;
	delete visManager;
	delete gopts;
	return EXIT_SUCCESS;
}
