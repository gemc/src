// geant4
#include "G4VisExecutive.hh"
#include "G4RunManagerFactory.hh"
#include "FTFP_BERT.hh"

// g4display
#include "g4SceneProperties.h"
#include "g4display.h"
#include "g4display_options.h"

// gemc
#include "glogger.h"

// qt
#include <QApplication>
#include <QMainWindow>

int main(int argc, char* argv[]) {
	// Initialize options and logging
	auto gopts = new GOptions(argc, argv, g4display::defineOptions());
	auto log   = new GLogger(gopts, G4DISPLAY_LOGGER);
	log->info(0, "g4display", "Starting g4display example...", gopts);

	// Optional GUI setup (only if --gui is passed)
	QApplication* app = nullptr;
	QMainWindow* window = nullptr;
	G4Display* g4display = nullptr;

	if (gopts->getSwitch("gui")) {
		log->info(0, "g4display", "Running in GUI mode...");
		app = new QApplication(argc, argv);
		window = new QMainWindow();
		window->setWindowTitle(QString::fromUtf8("displayUI example"));
	}

	auto visManager = new G4VisExecutive;
	visManager->Initialize();

	auto g4SceneProperties = new G4SceneProperties(gopts);

	// If GUI, show window and run Qt loop
	if (gopts->getSwitch("gui")) {

		g4display = new G4Display(gopts, window);
		window->setCentralWidget(g4display);
		window->show();
		int appResult = QApplication::exec();

		// Clean up GUI resources
		delete g4display;
		delete window;
		delete app;
		app = nullptr;

		// Clean up Geant4 and custom logic
		delete g4SceneProperties;
		delete visManager;
		delete log;
		delete gopts;

		return appResult;
	}

	// CLI mode
	log->info(0, "g4display", "Running in command line mode...");

	// (Optionally) Add custom CLI execution logic here

	// Clean up (CLI path)
	delete g4SceneProperties;
	delete visManager;
	delete log;
	delete gopts;

	return EXIT_SUCCESS;
}
