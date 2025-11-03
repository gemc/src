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


int main(int argc, char* argv[]) {
	auto gopts = std::make_shared<GOptions>(argc, argv, dbselect::defineOptions());
	auto log   = std::make_shared<GLogger>(gopts, SFUNCTION_NAME, DBSELECT_LOGGER);

	log->info(0, "dbselect", "Starting dbselect example...");

	// Optional GUI setup (only if --gui is passed)
	QApplication* app    = nullptr;
	QMainWindow*  window = nullptr;

	auto runManager = G4RunManagerFactory::CreateRunManager(G4RunManagerType::Default);
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
