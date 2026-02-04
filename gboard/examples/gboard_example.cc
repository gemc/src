// geant4
#include "G4VisExecutive.hh"
#include "G4RunManagerFactory.hh"

// gboard
#include "gboard.h"
#include "gui_session.h"

// qt
#include <QApplication>
#include <QMainWindow>
#include <QTimer>

/**
 * @file gboard_example.cc
 * @brief Example program showing how to route Geant4 output into a GBoard widget.
 *
 * This example demonstrates two modes:
 * - **GUI mode** (enabled with \c --gui): creates a \c QApplication, shows a \c QMainWindow containing GBoard,
 *   and installs a GUI_Session so Geant4 output is forwarded to the board.
 * - **CLI mode** (default): runs without creating Qt objects and simply exits after setup messages.
 *
 * The example also supports a timeout (scalar option \c tt) that automatically quits the Qt event loop
 * after the specified duration.
 *
 * Key behaviors illustrated:
 * - Creating module options via \c gboard::defineOptions().
 * - Constructing a GBoard owned by the Qt parent (the main window).
 * - Creating a GUI_Session that forwards Geant4 output to the board (session does not own the board).
 * - Running and terminating a Qt event loop using \c QTimer.
 */

/**
 * @brief Entry point for the gboard example application.
 *
 * Responsibilities:
 * - Initializes options and logging.
 * - Optionally initializes Qt GUI objects and shows a window that embeds GBoard.
 * - Initializes a Geant4 visualization manager to ensure Geant4 subsystems are active for the demo.
 * - In GUI mode, installs GUI_Session to route Geant4 UI output to the board.
 * - Exits either after the Qt event loop ends (GUI) or immediately (CLI).
 *
 * @param argc Standard C/C++ argument count.
 * @param argv Standard C/C++ argument vector.
 * @return \c EXIT_SUCCESS on normal completion, otherwise a non-zero status.
 */
int main(int argc, char* argv[]) {
	// Initialize options and logging.
	// The options structure is shared across module components via shared_ptr.
	auto gopts   = std::make_shared<GOptions>(argc, argv, gboard::defineOptions());
	auto log     = std::make_shared<GLogger>(gopts, SFUNCTION_NAME, GBOARD_LOGGER);
	auto gui     = gopts->getSwitch("gui");
	auto timeout = gopts->getScalarDouble("tt");
	int  ret     = EXIT_SUCCESS;

	log->info(0, "Starting gboard example...");

	// Optional GUI setup (only if --gui is passed).
	// Note: We allocate Qt objects dynamically here because the example explicitly deletes them.
	QApplication* app    = nullptr;
	QMainWindow*  window = nullptr;

	if (gui) {
		log->info(0, "gboard", "Running in GUI mode...");
		app    = new QApplication(argc, argv);
		window = new QMainWindow();
		window->setWindowTitle(QString::fromUtf8("displayUI example"));
	}

	// Initialize Geant4 visualization manager.
	// This is included to resemble the typical environment where Geant4 produces UI output.
	auto visManager = new G4VisExecutive;
	visManager->Initialize();

	// If GUI, show the window and run Qt loop.
	if (gui) {
		// GBoard is parented to the window, but the example explicitly deletes it at the end as well.
		auto* gboard = new GBoard(gopts, window);

		// GUI_Session installs itself as Geant4 cout destination and forwards to the board.
		// The session does not own the board.
		auto gui_session = std::make_unique<GUI_Session>(gopts, gboard);

		window->setCentralWidget(gboard);
		window->show();

		// Quit after timeout: this demonstrates deterministic shutdown for automated runs.
		QTimer::singleShot(timeout, [] {
			QCoreApplication::quit(); // stop the event loop
		});

		ret = QApplication::exec();

		delete gboard;
		delete window;
		delete app;
	}
	else {
		// CLI mode: no Qt objects are created.
		log->info(0, "Running gboard in command line mode...");
	}

	delete visManager;

	return ret;
}
