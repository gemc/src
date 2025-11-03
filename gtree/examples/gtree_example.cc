// geant4
#include "G4VisExecutive.hh"
#include "G4RunManagerFactory.hh"
#include "QBBC.hh"

// gtree
#include "gtree.h"
#include "gtree_options.h"

// gemc
#include "gdetectorConstruction.h"

// qt
#include <QApplication>
#include <QMainWindow>
#include <QTimer>


int main(int argc, char* argv[]) {
    // Initialize options and logging
    auto gopts = std::make_shared<GOptions>(argc, argv, gtree::defineOptions());
    auto log = std::make_shared<GLogger>(gopts, SFUNCTION_NAME, GTREE_LOGGER);
    auto timeout = gopts->getScalarDouble("tt");

    log->info(0, "Starting gtree example...");

    // Optional GUI setup (only if --gui is passed)
    QApplication* app = nullptr;
    QMainWindow* window = nullptr;

    auto runManager = G4RunManagerFactory::CreateRunManager(G4RunManagerType::Default);
    auto physicsList = new QBBC;
    runManager->SetUserInitialization(physicsList);

    auto visManager = new G4VisExecutive;
    visManager->Initialize();

    if (gopts->getSwitch("gui")) {
        log->info(0, "gtree", "Running in GUI mode...");
        app = new QApplication(argc, argv);
        window = new QMainWindow();
        window->setWindowTitle(QString::fromUtf8("gtree example"));
    }

    auto gdetector = new GDetectorConstruction(gopts);
    runManager->SetUserInitialization(gdetector);
    runManager->Initialize();

    auto g4vmap = gdetector->get_g4volumes_map();

    log->info(0, "gtree", "Detector construction successful with ", g4vmap.size(), " volumes.");

    if (gopts->getSwitch("gui")) {

        auto gtree = new GTree(gopts, g4vmap,  window);

        window->setCentralWidget(gtree);
        window->show();

        /* ---------- quit after 0.5s ---------- */
        // QTimer::singleShot(timeout, [] {
        //     QCoreApplication::quit(); // stop the event loop
        // });
        QTimer::singleShot(timeout, app, &QCoreApplication::quit); // ⬅️ key line :contentReference[oaicite:0]{index=0}

        int appResult = QApplication::exec();

        // Clean up GUI resources
        delete gtree;
        delete window;
        delete app;

        // Clean up Geant4 and custom logic
        delete visManager;

        return appResult;
    }


    // CLI mode
    log->info(0, "Running gtree in command line mode...");

    return EXIT_SUCCESS;
}
