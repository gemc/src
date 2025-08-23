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

	QApplication app(argc, argv);

	auto gopts =std::make_shared<GOptions>(argc, argv, gboard::defineOptions());
	auto log = std::make_shared<GLogger>(gopts, SFUNCTION_NAME, GBOARD_LOGGER);

	auto visManager = new G4VisExecutive;
	visManager->Initialize();

    // main window and controls
	auto window = new QMainWindow();
	window->setWindowTitle(QString::fromUtf8("displayUI example"));

	auto* gboard = new GBoard(gopts, window);
	window->setCentralWidget(gboard);

	auto gui_session = std::make_unique<GUI_Session>(gopts, gboard);

	log->info(0, "gboard example started");
	int ret = EXIT_SUCCESS;

	if (gopts->getSwitch("gui")) {
		window->show();

		// --- quit after 0.5 s ---
		QTimer::singleShot(500, &app, &QCoreApplication::quit);  // ⬅️ key line :contentReference[oaicite:0]{index=0}

		ret = QApplication::exec();    // returns when the timer fires
	}

	delete gboard;
	delete window;
	delete visManager;

	return ret;

}
