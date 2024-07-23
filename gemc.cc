// c++
#include <iostream>

using namespace std;

// geant4
#include "G4UImanager.hh"
#include "G4UIsession.hh"
#include "G4RunManagerFactory.hh"
#include "G4VisExecutive.hh"
#include "G4UIQt.hh"

// gemc
#include "goptions.h"
#include "gsplash.h"
#include "eventDispenser.h"
#include "g4SceneProperties.h"
#include "gphysics.h"
#include "gemcUtilities.h"
#include "gemcConventions.h"
#include "gemcOptions.h"
#include "gActionInitialization.h"
#include "gui.h"
#include "gsession.h"
#include "gdetectorConstruction.h"

int main(int argc, char *argv[]) {
    // the gemc goptions are defined in utilities/defineOptions.cc
    // they are merged with the frameworks goptions definitions
    // the goptions are then assigned from the jcard(s) and command line
    GOptions *gopts = new GOptions(argc, argv, gemc::defineOptions());


    // todo: add geant4 version here, see phys list on how

    // get gui switch, overlaps check and verbosity
    bool gui = gopts->getSwitch("gui");
    int checkForOverlaps = gopts->getScalarInt("checkOverlaps");
    int verbosity = gopts->getVerbosityFor("general");

    // splash screen
    GSplash *gemcSplash = nullptr;

    // createQtApplication returns a QApplication if gui is not zero
    // otherwise it returns a QCoreApplication
    QCoreApplication *gApp = createQtApplication(argc, argv, gui);
    if (gui) {
        gemcSplash = new GSplash("gemcArchitecture");
    }

    // instantiating new User Interface Messenger
    // our custom cout destination for the UIM: MasterGeant4.[log, err]
    G4UImanager *UIM = G4UImanager::GetUIpointer();
    UIM->SetCoutDestination(new GSession);

    // init geant4 run manager with number of threads coming from options
    // as of 11.2 the default is  task-based parallel mode
    auto runManager = G4RunManagerFactory::CreateRunManager(G4RunManagerType::Default);
    runManager->SetNumberOfThreads(getNumberOfThreads(gopts));

    // random engine
    startRandomEngine(gopts);

    // instantiating pointer to global digitization map
    // the map will be filled with the gsystem information of the sensitive detectors
    // the map is also used by eventDispenser to reload constants at every run number
    map < string, GDynamicDigitization * > *globalDigitizationMap = new map<string, GDynamicDigitization *>;

    // building detector
    // this is global, changed at main scope
    GDetectorConstruction *gDetectorGlobal = new GDetectorConstruction(gopts, globalDigitizationMap);
    runManager->SetUserInitialization(gDetectorGlobal);

    // starting gphysics
    auto gphysics = new GPhysics(gopts);

    // if showAvailablePhysicsX switch is on, print available physics and exit
    if (gopts->getSwitch("showAvailablePhysicsX")) {
        delete globalDigitizationMap;
        delete gApp;
        if (gui) {
            delete gemcSplash;
        }
        delete gopts;
        // Free the store: user actions, physics_list and detector_description are
        // owned and deleted by the run manager
        delete runManager;
        return EXIT_SUCCESS;
    }
    runManager->SetUserInitialization(gphysics->getPhysList());

    // instantiate GActionInitialization and initialize the geant4 kernel
    runManager->SetUserInitialization(new GActionInitialization(gopts, globalDigitizationMap));

    // this initializes g4MTRunManager, which:
    // calls Construct in GDetectorConstruction
    // calls ConstructSDandField in GDetectorConstruction
    // which in turns builds the gsystems, the g4systems, and the sensitive detectors in each thread,
    initGemcG4RunManager(runManager, gopts);

    // after the detectors are built, query the digitization names and load the plugins
    // maybe this should be done in GDetectorConstruction
    loadDigitizationPlugins(gopts, gDetectorGlobal->getDigitizationNamesList(), globalDigitizationMap);

    // G4VisExecutive can take a verbosity argument - see /vis/verbose guidance
    // notice we initialize this in batch mode as well
    G4VisManager *visManager = new G4VisExecutive("Quiet");
    visManager->Initialize();

    auto geventDispenser = new EventDispenser(gopts, globalDigitizationMap);


    if (gui) {
        // initializing qt session
        gemcSplash->message("Starting GUI");
        qApp->processEvents();

        // passing executable to retrieve full path
        GemcGUI gemcGui(argv[0], gopts, geventDispenser);
        gemcGui.show();
        gemcSplash->finish(&gemcGui);

        // intializing G4UIQt session
        G4UIsession *session = new G4UIQt(1, argv);

        // set display properties
        G4SceneProperties *g4SceneProperties = new G4SceneProperties(gopts);

        applyInitialUIManagerCommands(true, checkForOverlaps, verbosity);

        qApp->exec();

        // order of pointers deletion is inverse of creation
        delete g4SceneProperties;
        delete session;

    } else {
        // set display properties in batch mode
        G4SceneProperties *g4SceneProperties = new G4SceneProperties(gopts);
        applyInitialUIManagerCommands(false, checkForOverlaps, verbosity);
        geventDispenser->processEvents();

        delete g4SceneProperties;
    }

    // clearing pointers
    // delete visManager; deleting this cause error. Perhaps can define / delete in the functions above
    delete geventDispenser;

    for (auto [key, value]: (*globalDigitizationMap)) { delete value; }
    delete globalDigitizationMap;
    delete gApp;
    delete gopts;

    // Free the store: user actions, physics_list and detector_description are
    // owned and deleted by the run manager
    delete runManager;

    cout << GEMCLOGMSGITEM << KGRN << "Simulation completed, arrivederci! " << RST << endl << endl;
    return EXIT_SUCCESS;
}
