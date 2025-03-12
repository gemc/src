// geant4
#include "G4VisExecutive.hh"
#include "G4RunManagerFactory.hh"
#include "FTFP_BERT.hh"
#include "G4StepLimiterPhysics.hh"

// g4display
#include "g4SceneProperties.h"
#include "g4display.h"
#include "g4displayOptions.h"

// c++
#include <iostream>
using namespace std;

// qt
#include <QApplication>
#include <QMainWindow>

int main(int argc, char* argv[])
{
    GOptions *gopts = new GOptions(argc, argv, g4display::defineOptions());

    QApplication app(argc, argv);

    G4VisManager* visManager = new G4VisExecutive;
    visManager->Initialize();

    G4SceneProperties *g4SceneProperties = new G4SceneProperties(gopts);

    // main window
    QMainWindow *window = new QMainWindow();
    window->setWindowTitle(QString::fromUtf8("displayUI example"));

    // controls
    G4Display *g4display = new G4Display(gopts, window);
    window->setCentralWidget(g4display);

    if (gopts->getSwitch("gui")) {
        window->show();
        return app.exec();
    }

    delete g4display;
    delete window;
    delete g4SceneProperties;
    delete visManager;
    delete gopts;
    return EXIT_SUCCESS;

}
