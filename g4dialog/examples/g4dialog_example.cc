// geant4
#include "G4VisExecutive.hh"
#include "G4RunManagerFactory.hh"

// g4dialog
#include "g4dialog.h"
#include "g4dialog_options.h"

// qt
#include <QApplication>
#include <QMainWindow>
#include <QTimer>

int main(int argc, char* argv[]) {
	// Initialize options and logging
	auto gopts   = std::make_shared<GOptions>(argc, argv, g4dialog::defineOptions());
	auto log     = std::make_shared<GLogger>(gopts, SFUNCTION_NAME, G4DIALOG_LOGGER);
	auto gui     = gopts->getSwitch("gui");
	auto timeout = gopts->getScalarDouble("tt");
	int  ret     = EXIT_SUCCESS;

	log->info(0, "Starting g4dialog example...");

	// Optional GUI setup (only if --gui is passed)
	QApplication* app    = nullptr;
	QMainWindow*  window = nullptr;

	if (gui) {
		log->info(0, "g4dialog", "Running in GUI mode...");
		app    = new QApplication(argc, argv);
		window = new QMainWindow();
		window->setWindowTitle(QString::fromUtf8("displayUI example"));
	}

	auto visManager = new G4VisExecutive;
	visManager->Initialize();

	// If GUI, show the window and run Qt loop
	if (gui) {
		auto *g4dialog = new G4Dialog(gopts, window);
		window->setCentralWidget(g4dialog);
		window->show();

		// quit after timeout
		QTimer::singleShot(timeout, [] {
			QCoreApplication::quit(); // stop the event loop
		});

		ret = QApplication::exec();

		delete g4dialog;
		delete window;
		delete app;
	}
	else {
		// CLI mode
		log->info(0, "Running g4dialog in command line mode...");
	}

	delete visManager;

	return ret;
}
