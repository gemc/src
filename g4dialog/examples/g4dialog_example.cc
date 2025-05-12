// geant4
#include "G4VisExecutive.hh"
#include "G4RunManagerFactory.hh"

// g4dialog
#include "g4dialog.h"
#include "g4dialog_options.h"

// qt
#include <QApplication>
#include <QMainWindow>

int main(int argc, char* argv[]) {

	auto gopts = new GOptions(argc, argv, g4dialog::defineOptions());
	auto log = std::make_shared<GLogger>(gopts, G4DIALOG_LOGGER, "g4dialog example");

	QApplication app(argc, argv);

	auto visManager = new G4VisExecutive;
	visManager->Initialize();

	// main window
	auto window = new QMainWindow();
	window->setWindowTitle(QString::fromUtf8("displayUI example"));

	// controls
	auto g4dialog = new G4Dialog(gopts, window);
	window->setCentralWidget(g4dialog);


	log->info(0, "g4 dialog example started");

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
