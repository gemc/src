/**
 * \file g4display_example.cc
 * \brief Minimal example program demonstrating g4display usage.
 *
 * This example shows how to:
 * - construct a GOptions instance using g4display::defineOptions,
 * - initialize a module logger (Glogger),
 * - initialize Geant4 visualization (\c G4VisExecutive),
 * - create \ref G4Display and embed it into a \c QMainWindow when GUI mode is enabled.
 *
 * The example supports both GUI and CLI flows:
 * - GUI flow is enabled by passing the \c --gui switch.
 * - An optional timeout (option \c tt) can be used to auto-quit the Qt event loop.
 *
 */

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
	// Initialize options and logging.
	// GOptions is configured with the g4display option schema, which includes view/camera/dawn/text options.
	auto gopts   = std::make_shared<GOptions>(argc, argv, g4display::defineOptions());
	auto log     = std::make_shared<GLogger>(gopts, SFUNCTION_NAME, G4DISPLAY_LOGGER);
	auto gui     = gopts->getSwitch("gui");
	auto timeout = gopts->getScalarDouble("tt");
	int  ret     = EXIT_SUCCESS;

	log->info(0, "Starting g4display example...");

	// Optional GUI setup (only if --gui is passed).
	// Qt objects are created dynamically to keep CLI mode lightweight.
	QApplication* app    = nullptr;
	QMainWindow*  window = nullptr;

	if (gui) {
		log->info(0, "g4display", "Running in GUI mode...");
		app    = new QApplication(argc, argv);
		window = new QMainWindow();
		window->setWindowTitle(QString::fromUtf8("g4display example"));
	}

	// Initialize Geant4 visualization manager.
	auto visManager = new G4VisExecutive;
	visManager->Initialize();

	// Build scene properties helper (command list generation based on options).
	auto g4SceneProperties = new G4SceneProperties(gopts);

	// If GUI, show the window and run Qt loop.
	if (gui) {
		//! [g4display_example_core]
		auto g4display = new G4Display(gopts, window);
		window->setCentralWidget(g4display);
		window->show();
		//! [g4display_example_core]

		// Quit after timeout (milliseconds). This allows automated tests/demos without manual close.
		QTimer::singleShot(timeout, [] {
			QCoreApplication::quit(); // stop the event loop
		});

		ret = QApplication::exec();

		// Clean up GUI resources.
		delete g4display;
		delete window;
		delete app;
	}
	else {
		// CLI mode: the example currently only initializes infrastructure and exits.
		// A future CLI extension could apply g4SceneProperties->scene_commands(...) to the Geant4 UI manager.
		log->info(0, "Running g4display in command line mode...");
	}

	delete g4SceneProperties;
	delete visManager;

	return ret;
}
