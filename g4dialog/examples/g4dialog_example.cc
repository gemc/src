/**
 * \file g4dialog_example.cc
 *
 * \anchor g4dialog_example_main
 *
 * \brief Example program demonstrating how to instantiate and run G4Dialog.
 *
 * \details
 * This example supports two modes:
 * - **GUI mode** (enabled by \c --gui): creates a \c QApplication, embeds G4Dialog in a \c QMainWindow,
 *   and runs the Qt event loop. A timer can be used to auto-quit after a configurable timeout.
 * - **CLI mode** (default): initializes options/logging and performs minimal setup without starting Qt.
 *
 * The example also initializes a \c Geant4 visualization manager (\c G4VisExecutive) so that UI commands
 * and visualization-related help are available in typical setups.
 *
 * Notes:
 * - The exact options (e.g., \c --gui, \c --tt) are expected to be provided by the surrounding option
 *   definition conventions via g4dialog::defineOptions().
 * - This example intentionally keeps the logic minimal and focused on wiring the UI components together.
 */

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

/**
 * \brief Program entry point for the G4Dialog example.
 *
 * \param argc Number of command-line arguments.
 * \param argv Command-line argument vector.
 * \return Process exit code (\c EXIT_SUCCESS on normal completion).
 */
int main(int argc, char* argv[]) {
	// Initialize options and logging (framework-provided).
	auto gopts   = std::make_shared<GOptions>(argc, argv, g4dialog::defineOptions());
	auto log     = std::make_shared<GLogger>(gopts, SFUNCTION_NAME, G4DIALOG_LOGGER);
	auto gui     = gopts->getSwitch("gui");
	auto timeout = gopts->getScalarDouble("tt");
	int  ret     = EXIT_SUCCESS;

	log->info(0, "Starting g4dialog example...");

	// Optional GUI setup (only if --gui is passed).
	QApplication* app    = nullptr;
	QMainWindow*  window = nullptr;

	if (gui) {
		log->info(0, "g4dialog", "Running in GUI mode...");
		app    = new QApplication(argc, argv);
		window = new QMainWindow();
		window->setWindowTitle(QString::fromUtf8("displayUI example"));
	}

	// Initialize Geant4 visualization manager so that UI/vis-related commands are available.
	auto visManager = new G4VisExecutive;
	visManager->Initialize();

	// If GUI, show the window and run Qt loop.
	if (gui) {
		auto* g4dialog = new G4Dialog(gopts, window);
		window->setCentralWidget(g4dialog);
		window->show();

		// Quit after timeout (useful for automated testing or demos).
		QTimer::singleShot(timeout, [] {
			QCoreApplication::quit(); // stop the event loop
		});

		ret = QApplication::exec();

		// Explicit cleanup (Qt parent ownership covers most UI elements, but keep this obvious here).
		delete g4dialog;
		delete window;
		delete app;
	}
	else {
		// CLI mode: no Qt loop is started; this path is useful as a smoke test for options/logging setup.
		log->info(0, "Running g4dialog in command line mode...");
	}

	// Cleanup Geant4 visualization manager.
	delete visManager;

	return ret;
}
