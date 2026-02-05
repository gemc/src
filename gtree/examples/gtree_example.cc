/**
 * @file gtree_example.cc
 * @anchor gtree_example
 * @ingroup gtree_module
 * @brief Example application demonstrating how to instantiate and run the GTree widget.
 *
 * @details
 * This example shows a minimal workflow:
 * - Build an option set using `gtree::defineOptions()`
 * - Construct Geant4 run manager and visualization manager
 * - Build the detector construction and retrieve the geometry volume map
 * - If `--gui` is enabled, create a Qt main window and embed GTree as the central widget
 * - Exit after a configurable timeout
 *
 * **How to run (conceptual)**
 * - GUI mode: pass `--gui` to create the Qt event loop and show the widget.
 * - Timeout: the example reads a scalar double option named `tt` and uses it as the
 *   delay (in milliseconds) before quitting the Qt event loop.
 *
 * The option names (`gui`, `tt`) are expected to be defined by the composed option set
 * returned by `gtree::defineOptions()`.
 */

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


/**
 * @brief Example program entry point that launches the geometry tree browser.
 *
 * @param argc Standard command line argument count.
 * @param argv Standard command line argument vector.
 * @return Process exit code. In GUI mode this is the Qt application return code.
 *
 * @details
 * Execution outline:
 * 1. Parse options and configure logging.
 * 2. Create Geant4 run manager, physics list, and visualization manager.
 * 3. Create the detector and initialize the run manager.
 * 4. Retrieve the geometry volume map from the detector construction.
 * 5. If `--gui` is enabled:
 *    - Create the Qt application and main window.
 *    - Create a GTree widget using the volume map.
 *    - Quit the Qt event loop after the configured timeout.
 * 6. Otherwise (CLI mode), exit after initialization.
 */
int main(int argc, char* argv[]) {
    // Initialize options and logging
    auto gopts = std::make_shared<GOptions>(argc, argv, gtree::defineOptions());
    auto log = std::make_shared<GLogger>(gopts, SFUNCTION_NAME, GTREE_LOGGER);
    auto timeout = gopts->getScalarDouble("tt");

    log->info(0, "Starting gtree example...");

    // Optional GUI setup (only if --gui is passed)
    QApplication* app = nullptr;
    QMainWindow* window = nullptr;

    // Geant4: create a default run manager and register a reference physics list.
    auto runManager = G4RunManagerFactory::CreateRunManager(G4RunManagerType::Default);
    auto physicsList = new QBBC;
    runManager->SetUserInitialization(physicsList);

    // Geant4: visualization manager used by geometry UI commands invoked by the widget.
    auto visManager = new G4VisExecutive;
    visManager->Initialize();

    if (gopts->getSwitch("gui")) {
        log->info(0, "gtree", "Running in GUI mode...");
        app = new QApplication(argc, argv);
        window = new QMainWindow();
        window->setWindowTitle(QString::fromUtf8("gtree example"));
    }

    // Build detector and initialize the run manager so geometry exists.
    auto gdetector = new GDetectorConstruction(gopts);
    runManager->SetUserInitialization(gdetector);
    runManager->Initialize();

    // Retrieve volume map used by GTree to build the system/volume hierarchy.
    auto g4vmap = gdetector->get_g4volumes_map();

    log->info(0, "gtree", "Detector construction successful with ", g4vmap.size(), " volumes.");

    if (gopts->getSwitch("gui")) {

        auto gtree = new GTree(gopts, g4vmap,  window);

        window->setCentralWidget(gtree);
        window->show();

        // Quit after the configured delay (milliseconds) to make the example self-terminating.
        QTimer::singleShot(timeout, app, &QCoreApplication::quit);

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
