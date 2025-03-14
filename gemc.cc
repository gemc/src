// c++
#include <iostream>

using namespace std;

// geant4
#include "G4UImanager.hh"
#include "G4UIsession.hh"
#include "G4RunManagerFactory.hh"
#include "G4VisExecutive.hh"
#include "G4UIQt.hh"
//#include "G4Transportation.hh"
#include "G4TransportationParameters.hh"

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
    // QScopedPointer os smart pointer in Qt that manages the lifetime of a QCoreApplication
    QScopedPointer <QCoreApplication> gApp(createQtApplication(argc, argv, gui));

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


    G4double warningE = 10.0 * CLHEP::keV;
    G4double importantE = 1 * CLHEP::GeV;
    G4int numTrials = 30;

    auto transportParams = G4TransportationParameters::Instance();
    transportParams->SetWarningEnergy(warningE);
    transportParams->SetImportantEnergy(importantE);
	transportParams->SetNumberOfTrials(numTrials);

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
	cout << GEMCLOGMSGITEM << KMAG << "Tally summary: " << gDetectorGlobal->get_number_of_volumes() << " volumes, " << gDetectorGlobal->get_number_of_g4_volumes() << " geant4 built volumes" << RST << endl << endl;


    if (gui) {
        // initializing qt session
        gemcSplash->message("Starting GUI");
        qApp->processEvents();

        // intializing G4UIQt session. Notice g4SceneProperties has to be declared after this, so we have to duplicate code below.
        G4UIsession *uiQtSession = new G4UIQt(1, argv);
        G4SceneProperties *g4SceneProperties = new G4SceneProperties(gopts);

        // passing executable to retrieve full path
        GemcGUI gemcGui(argv[0], gopts, geventDispenser);
        gemcGui.show();
        gemcSplash->finish(&gemcGui);

        if (gDetectorGlobal->is_empty()) {
            cout << GEMCLOGMSGITEM << "Warning: Detector is empty. Nothing to do." << endl;
        } else {
            applyInitialUIManagerCommands(true, checkForOverlaps, verbosity);

            qApp->exec();
        }

        delete g4SceneProperties;
        delete uiQtSession;

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
    delete gopts;

    // Free the store: user actions, physics_list and detector_description are
    // owned and deleted by the run manager
    delete runManager;

    cout << GEMCLOGMSGITEM << KGRN << "Simulation completed, arrivederci! " << RST << endl << endl;
    return EXIT_SUCCESS;
}
