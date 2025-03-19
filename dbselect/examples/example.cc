// dbselect
#include "dbselectView.h"
#include "dbselectOptions.h"

// c++
#include <iostream>

using namespace std;

// qt
#include <QApplication>
#include <QMainWindow>

int main(int argc, char *argv[]) {
	GOptions *gopts = new GOptions(argc, argv, dbselect::defineOptions());

	QApplication app(argc, argv);

	map < string, GDynamicDigitization * > *globalDigitizationMap = new map<string, GDynamicDigitization *>;

	// building detector
	// this is global, changed at main scope
	// this will also load the digitization plugins
	GDetectorConstruction *gDetectorGlobal = new GDetectorConstruction(gopts, globalDigitizationMap);


	// main window
	QMainWindow *window = new QMainWindow();
	window->setWindowTitle(QString::fromUtf8("displayUI example"));

	// controls
	DBSelectView *dbselect = new DBSelectView(gopts, gDetectorGlobal, window);
	window->setCentralWidget(dbselect);

	if (gopts->getSwitch("gui")) {
		window->show();
		return app.exec();
	}

	delete dbselect;
	delete window;
	delete gopts;
	return EXIT_SUCCESS;

}
