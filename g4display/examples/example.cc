// geant4
#include "G4VisExecutive.hh"


// g4display
#include "g4SceneProperties.h"
#include "g4controls.h"
#include "g4displayOptions.h"

// c++
#include <iostream>
using namespace std;

// qt
#include <QApplication>
#include <QMainWindow>

int main(int argc, char* argv[])
{

	QApplication app(argc, argv);

	GOptions *gopts = new GOptions(argc, argv, g4display::defineOptions());

	G4VisManager* visManager = new G4VisExecutive;
	visManager->Initialize();

	G4SceneProperties *g4SceneProperties = new G4SceneProperties(gopts);

	// main window
	QMainWindow *window = new QMainWindow();
	window->setWindowTitle(QString::fromUtf8("displayUI example"));

	// controls
	G4Controls *g4controls = new G4Controls(gopts, window);
	window->setCentralWidget(g4controls);


	window->show();
	delete g4SceneProperties;

    if (gopts->getSwitch("gui")) {
        return app.exec();
    }

    return EXIT_SUCCESS;

}
