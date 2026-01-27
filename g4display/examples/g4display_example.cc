// geant4
#include "G4VisExecutive.hh"
#include "G4RunManagerFactory.hh"

// g4display
#include "g4SceneProperties.h"
#include "g4display.h"
#include "g4display_options.h"

// gemc
#include "glogger.h"

// qt
#include <QApplication>
#include <QMainWindow>
#include <QTimer>

int main(int argc, char* argv[]) {
	// Initialize options and logging
	auto gopts   = std::make_shared<GOptions>(argc, argv, g4display::defineOptions());
	auto log     = std::make_shared<GLogger>(gopts, SFUNCTION_NAME, G4DISPLAY_LOGGER);
	auto gui     = gopts->getSwitch("gui");
	auto timeout = gopts->getScalarDouble("tt");
	int  ret     = EXIT_SUCCESS;

	log->info(0, "Starting g4display example...");

	// Optional GUI setup (only if --gui is passed)
	QApplication* app    = nullptr;
	QMainWindow*  window = nullptr;

	if (gui) {
		log->info(0, "g4display", "Running in GUI mode...");
		app    = new QApplication(argc, argv);
		window = new QMainWindow();
		window->setWindowTitle(QString::fromUtf8("g4display example"));
	}

	auto visManager = new G4VisExecutive;
	visManager->Initialize();

	auto g4SceneProperties = new G4SceneProperties(gopts);

	// If GUI, show the window and run Qt loop
	if (gui) {
		auto g4display = new G4Display(gopts, window);
		window->setCentralWidget(g4display);
		window->show();

		// quit after timeout
		QTimer::singleShot(timeout, [] {
			QCoreApplication::quit(); // stop the event loop
		});

		ret = QApplication::exec();

		// Clean up GUI resources
		delete g4display;
		delete window;
		delete app;
	}
	else {
		// CLI mode
		log->info(0, "Running g4display in command line mode...");
	}

	delete g4SceneProperties;
	delete visManager;

	return ret;
}
