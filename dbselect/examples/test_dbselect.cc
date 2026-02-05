/**
 * @file test_dbselect.cc
 * @ingroup dbselect_module
 *
 * @anchor test_dbselect
 *
 * @brief Example program demonstrating how to use the dbselect module in GUI or CLI mode.
 *
 * Summary:
 * This example creates a GOptions instance using dbselect::defineOptions(), initializes
 * logging for the \c "dbselect" logger, constructs a \c GDetectorConstruction, and then:
 * - if \c --gui is enabled, creates a Qt window hosting \c DBSelectView and demonstrates a
 *   timed geometry reload and quit,
 * - otherwise runs in CLI mode (currently a minimal stub in this example).
 *
 * Typical usage:
 * \code
 * # GUI mode
 * test_dbselect --gui --sql my_geometry.sqlite --experiment clas12
 *
 * # CLI mode
 * test_dbselect --sql my_geometry.sqlite --experiment clas12
 * \endcode
 *
 * Notes:
 * - Qt types (e.g. \c QApplication, \c QMainWindow, \c QTimer) are used only when \c --gui is set.
 * - Geant4 initialization in this example uses \c G4RunManagerFactory and \c QBBC.
 */

// dbselect
#include "dbselectView.h"
#include "dbselect_options.h"

// qt
#include <QApplication>
#include <QMainWindow>
#include <QTimer>

// geant4
#include "G4RunManagerFactory.hh"
#include "QBBC.hh"


/**
 * @brief Program entry point for the dbselect example.
 *
 * Responsibilities:
 * - Build a shared GOptions configuration from argc/argv using dbselect::defineOptions().
 * - Initialize logging using the dbselect logger name.
 * - Set up a Geant4 run manager and a physics list (required by the detector construction path).
 * - If \c --gui is enabled:
 *   - start a Qt application,
 *   - show a main window hosting \c DBSelectView,
 *   - schedule a geometry reload and a timed quit via \c QTimer.
 * - Otherwise:
 *   - run in CLI mode (currently a minimal stub returning success).
 *
 * @param argc Standard command-line argument count.
 * @param argv Standard command-line argument vector.
 * @return Application exit code (Qt loop exit code in GUI mode, otherwise \c EXIT_SUCCESS).
 */
int main(int argc, char* argv[]) {
	auto gopts = std::make_shared<GOptions>(argc, argv, dbselect::defineOptions());
	auto log   = std::make_shared<GLogger>(gopts, SFUNCTION_NAME, DBSELECT_LOGGER);

	log->info(0, "dbselect", "Starting dbselect example...");

	// Optional GUI setup (only if --gui is passed)
	QApplication* app    = nullptr;
	QMainWindow*  window = nullptr;

	// Geant4 initialization (required before using detector construction in many setups).
	auto runManager  = G4RunManagerFactory::CreateRunManager(G4RunManagerType::Default);
	auto physicsList = new QBBC;
	runManager->SetUserInitialization(physicsList);

	if (gopts->getSwitch("gui")) {
		log->info(0, "Running dbselect in GUI mode...");
		app    = new QApplication(argc, argv);
		window = new QMainWindow();
		window->setWindowTitle(QString::fromUtf8("dbselect example"));
	}

	// building detector
	// this is global, changed at main scope
	// this will also load the digitization plugins
	auto gdetector = new GDetectorConstruction(gopts);

	// If GUI, show the window and run Qt loop
	if (gopts->getSwitch("gui")) {
		auto dbselect = new DBSelectView(gopts, gdetector, window);
		window->setCentralWidget(dbselect);
		window->show();

		/* ---------- testing reload_geometry after 0.5s ---------- */
		QTimer::singleShot(100, [dbselect] {
			dbselect->reload_geometry(); // stop the event loop
		});

		/* ---------- quit after 0.5s ---------- */
		QTimer::singleShot(500, []
		{
			QCoreApplication::quit();          // stop the event loop
		});

		int appResult = QApplication::exec();

		// Clean up GUI resources
		delete dbselect;
		delete window;
		delete app;

		return appResult;
	}

	// CLI mode
	log->info(0, "Running dbselect in command line mode...");

	return EXIT_SUCCESS;
}
