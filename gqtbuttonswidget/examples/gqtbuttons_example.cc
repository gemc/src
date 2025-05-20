// gQtButtonsWidget
#include "qapplication.h"
#include "gQtButtonsWidget.h"
#include "gQtButtonsWidget_options.h"

// qt
#include <QApplication>
#include <QMainWindow>
#include <QTimer>


int main(int argc, char* argv[]) {
	// options not used
	GOptions* gopts = new GOptions(argc, argv, gqtbuttonswidget::defineOptions());

	QApplication app(argc, argv);

	vector<string> bicons;

	// notice path must match the path in the qrc file
	bicons.push_back(":/images/firstButton");
	bicons.push_back(":/images/secondButton");

	int ret = EXIT_SUCCESS;

	if (gopts->getSwitch("gui")) {
		GQTButtonsWidget window(128, 128, bicons);
		window.show();
		// --- quit after 0.5 s ---
		QTimer::singleShot(500, &app, &QCoreApplication::quit); // ⬅️ key line :contentReference[oaicite:0]{index=0}

		ret = QApplication::exec(); // returns when the timer fires
	}

	return ret;
}
