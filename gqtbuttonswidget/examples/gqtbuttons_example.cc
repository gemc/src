// gQtButtonsWidget
//#include "qapplication.h"
#include "gQtButtonsWidget_options.h"
#include "gQtButtonsWidget.h"

// qt
#include <QApplication>
#include <QMainWindow>
#include <QTimer>

int main(int argc, char* argv[]) {
	// Initialize options and logging
	auto gopts   = std::make_shared<GOptions>(argc, argv, GOptions("hello"));
	auto gui     = gopts->getSwitch("gui");
	auto timeout = gopts->getScalarDouble("tt");
	int  ret     = EXIT_SUCCESS;

	// Optional GUI setup (only if --gui is passed)
	QApplication* app    = nullptr;

	if (gui) {
		app    = new QApplication(argc, argv);
	}

	// notice path must match the path in the qrc file
	std::vector<std::string> bicons;
	bicons.emplace_back(":/images/firstButton");
	bicons.emplace_back(":/images/secondButton");


	if (gui) {
		GQTButtonsWidget window(128, 128, bicons);
		window.show();

		// quit after timeout
		QTimer::singleShot(timeout, [] {
			QCoreApplication::quit(); // stop the event loop
		});

		ret = QApplication::exec();

		// Clean up GUI resources
		delete app;
	}
	else {
		// CLI mode
	}

	return ret;
}
