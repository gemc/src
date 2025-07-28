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
	auto gopts = std::make_shared<GOptions>(argc, argv, g4display::defineOptions());
	auto log   =  std::make_shared<GLogger>(gopts, G4DISPLAY_LOGGER, "g4display example");

	log->info(0, "Starting g4display example...", gopts);

	// Optional GUI setup (only if --gui is passed)
	QApplication* app = nullptr;
	QMainWindow* window = nullptr;

	if (gopts->getSwitch("gui")) {
		log->info(0, "g4display", "Running in GUI mode...");
		app = new QApplication(argc, argv);
		window = new QMainWindow();
		window->setWindowTitle(QString::fromUtf8("displayUI example"));
	}

	auto visManager = new G4VisExecutive;
	visManager->Initialize();

	auto g4SceneProperties = new G4SceneProperties(gopts.get());

	// If GUI, show the window and run Qt loop
	if (gopts->getSwitch("gui")) {

		auto g4display = new G4Display(gopts.get(), window);
		window->setCentralWidget(g4display);
		window->show();

		/* ---------- quit after 0.5s ---------- */
		QTimer::singleShot(500, []
		{
			QCoreApplication::quit();          // stop the event loop
		});

		int appResult = QApplication::exec();

		// Clean up GUI resources
		delete g4display;
		delete window;
		delete app;

		// Clean up Geant4 and custom logic
		delete g4SceneProperties;
		delete visManager;

		return appResult;
	}

	// CLI mode
	log->info(0, "Running g4display in command line mode...");


	delete g4SceneProperties;
	delete visManager;

	return EXIT_SUCCESS;
}
