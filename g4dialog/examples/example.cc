// geant4
#include "G4VisExecutive.hh"
#include "G4RunManagerFactory.hh"
#include "FTFP_BERT.hh"
#include "G4StepLimiterPhysics.hh"

// g4dialog
#include "g4dialog.h"
#include "g4dialogOptions.h"

// c++
#include <iostream>
using namespace std;

// qt
#include <QApplication>
#include <QMainWindow>

int main(int argc, char* argv[])
{
    GOptions *gopts = new GOptions(argc, argv, g4dialog::defineOptions());

    QApplication app(argc, argv);

    G4VisManager* visManager = new G4VisExecutive;
    visManager->Initialize();

    // main window
    QMainWindow *window = new QMainWindow();
    window->setWindowTitle(QString::fromUtf8("displayUI example"));

    // controls
    G4Dialog *g4dialog = new G4Dialog(gopts, window);
    window->setCentralWidget(g4dialog);

    if (gopts->getSwitch("gui")) {
        window->show();
        return app.exec();
    }

    delete g4dialog;
    delete window;
    delete visManager;
    delete gopts;
    return EXIT_SUCCESS;

}
