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

int main(int argc, char* argv[]) {
	// Initialize options and logging
	auto gopts   = std::make_shared<GOptions>(argc, argv, gboard::defineOptions());
	auto log     = std::make_shared<GLogger>(gopts, SFUNCTION_NAME, GBOARD_LOGGER);
	auto gui     = gopts->getSwitch("gui");
	auto timeout = gopts->getScalarDouble("tt");
	int  ret     = EXIT_SUCCESS;

	log->info(0, "Starting gboard example...");

	// Optional GUI setup (only if --gui is passed)
	QApplication* app    = nullptr;
	QMainWindow*  window = nullptr;

	if (gui) {
		log->info(0, "gboard", "Running in GUI mode...");
		app    = new QApplication(argc, argv);
		window = new QMainWindow();
		window->setWindowTitle(QString::fromUtf8("displayUI example"));
	}

	auto visManager = new G4VisExecutive;
	visManager->Initialize();

	// If GUI, show the window and run Qt loop
	if (gui) {
		auto* gboard      = new GBoard(gopts, window);
		auto  gui_session = std::make_unique<GUI_Session>(gopts, gboard);
		window->setCentralWidget(gboard);
		window->show();

		// quit after timeout
		QTimer::singleShot(timeout, [] {
			QCoreApplication::quit(); // stop the event loop
		});

		ret = QApplication::exec();

		delete gboard;
		delete window;
		delete app;
	}
	else {
		// CLI mode
		log->info(0, "Running gboard in command line mode...");
	}

	delete visManager;

	return ret;
}
