// Qt
#include <QtWidgets>

// geant4 headers
#include "G4Threading.hh"
#include "G4UImanager.hh"

// c++
#include <iostream>
using namespace std;

// gemc
#include "goptions.h"
#include "gemcConventions.h"
#include "gemcUtilities.h"

// distinguishing between graphical and batch mode
QCoreApplication* createQtApplication(int &argc, char *argv[], bool gui)
{
	if( gui ) {
		QFileInfo qrcFileInfoExecutable(argv[0]);
		QString rccPath = qrcFileInfoExecutable.absolutePath() + "/" + "qtresources.rcc";
		QResource::registerResource(rccPath);

		return new QApplication(argc, argv);
	} else {
		return new QCoreApplication(argc, argv);
	}
}

// return number of cores from options. If 0 or none given,
// returns max number of available cores
int getNumberOfThreads(GOptions* gopts) {
	int useThreads = gopts->getInt("nthreads");
	int allThreads = G4Threading::G4GetNumberOfCores();
	if(useThreads == 0) useThreads = allThreads;

	int verbosity = gopts->getInt("verbosity");

	// global log screen
	if (verbosity >= GVERBOSITY_SUMMARY) {
		cout << GEMCLOGMSGITEM << "G4MTRunManager: using " << useThreads << " threads out of "  << allThreads << " available."  << endl;
	}
	
	return useThreads;
}


// initialize G4MTRunManager
void initGemcG4RunManager(G4RunManager *grm, GOptions* gopts)
{
	int tlog = gopts->getInt("tlog");
    bool showG4ThreadsLog = gopts->getSwitch("showG4ThreadsLog");

	G4UImanager *g4uim   = G4UImanager::GetUIpointer();
    if ( ! showG4ThreadsLog ) { g4uim->ApplyCommand("/control/cout/setCoutFile gthread.log"); }
	
	if ( tlog != 0 ) {
		g4uim->ApplyCommand("/control/cout/ignoreThreadsExcept " + to_string(tlog));
	}
	// initialize run manager
	grm->Initialize();
}


vector<string> startingUIMCommands(bool gui, int checkForOverlaps) {

	vector<string> commands;
	
	// added additional overlaps check if set to 2
	// if set to a number greater than 100
	if ( checkForOverlaps == 2 ) {
		cout << GEMCLOGMSGITEM << "Running /geometry/test/run with 50 points. NOTICE: currently this fails" << endl;
		commands.push_back("/geometry/test/resolution 50" );
		commands.push_back("/geometry/test/run");
	} else if ( checkForOverlaps >= 100 ) {
		cout << GEMCLOGMSGITEM << "Running /geometry/test/run with " << to_string(checkForOverlaps) << " points. NOTICE: currently this fails. " << endl;
		commands.push_back("/geometry/test/resolution " + to_string(checkForOverlaps));
		commands.push_back("/geometry/test/run");
	}

	// not in gui mode, return batch only
	if( !gui ) return commands;

	// define gui commands
	// all this needs to go in scene properties
	// and gui needs not to be passed here 
	commands.push_back("/vis/scene/add/trajectories rich smooth");
	commands.push_back("/vis/scene/add/hits");
	commands.push_back("/vis/scene/endOfEventAction accumulate");        // for some reason refresh (default) won't work here
	commands.push_back("/vis/viewer/set/culling coveredDaughters true");
	commands.push_back("/vis/viewer/set/background 1 1 1 1");   // 205, 230, 251 = CD, E6, FA

	return commands;

}

// apply UIM commands
void applyInitialUIManagerCommands(bool gui, int checkForOverlaps, int verbosity) {
	G4UImanager *g4uim = G4UImanager::GetUIpointer();

	vector<string> commands = startingUIMCommands(gui, checkForOverlaps);

	for(auto &c : commands) {
		if(verbosity > GVERBOSITY_SUMMARY) {
			cout << GEMCLOGMSGITEM << "Executing UIManager command \"" << c << "\"" << endl;
		}
		g4uim->ApplyCommand(c.c_str());
	}
}


#include <unistd.h>  // needed for get_pid
#include "CLHEP/Random/DRand48Engine.h"
#include "CLHEP/Random/DualRand.h"
#include "CLHEP/Random/Hurd160Engine.h"
#include "CLHEP/Random/Hurd288Engine.h"
#include "CLHEP/Random/JamesRandom.h"
#include "CLHEP/Random/MixMaxRng.h"
#include "CLHEP/Random/MTwistEngine.h"
#include "CLHEP/Random/RandEngine.h"
#include "CLHEP/Random/RanecuEngine.h"
#include "CLHEP/Random/RanluxEngine.h"
#include "CLHEP/Random/Ranlux64Engine.h"
#include "CLHEP/Random/RanluxppEngine.h"
#include "CLHEP/Random/RanshiEngine.h"
#include "CLHEP/Random/TripleRand.h"

void startRandomEngine(GOptions* gopts) {

    string randomEngineName = gopts->getString("randomEngineName");
    int seed = gopts->getInt("randomSeed");
    if ( seed == -99 ) {
        double timed = time(NULL);
        double clockd = clock();
        double getpidi = (double) getpid();
        seed = (G4int) (  timed - clockd - getpidi );
    }

    cout << GEMCLOGMSGITEM << "Starting random engine: >" << randomEngineName << "< using seed: " << seed << endl;

    // the below will not work:
    // if uncommented, it will set the same seed for each event - even if setTheSeed is called below


    // the names come from the CLHEP library, can be found with
    // grep ": public HepRandomEngine" *.h $CLHEP_BASE_DIR/include/CLHEP/Random/* | awk -Fclass '{print $2}' | awk -F: '{print $1}'
//    if      ( randomEngineName == "DRand48Engine")  G4Random::setTheEngine(new CLHEP::DRand48Engine(seed));
//    else if ( randomEngineName == "DualRand")       G4Random::setTheEngine(new CLHEP::DualRand);
//    else if ( randomEngineName == "Hurd288Engine")  G4Random::setTheEngine(new CLHEP::Hurd160Engine);
//    else if ( randomEngineName == "HepJamesRandom") G4Random::setTheEngine(new CLHEP::HepJamesRandom);
//    else if ( randomEngineName == "MTwistEngine")   G4Random::setTheEngine(new CLHEP::MTwistEngine);
//    else if ( randomEngineName == "MixMaxRng")      G4Random::setTheEngine(new CLHEP::MixMaxRng(seed));
//    else if ( randomEngineName == "RandEngine")     G4Random::setTheEngine(new CLHEP::RandEngine);
//    else if ( randomEngineName == "RanecuEngine")   G4Random::setTheEngine(new CLHEP::RanecuEngine);
//    else if ( randomEngineName == "Ranlux64Engine") G4Random::setTheEngine(new CLHEP::Ranlux64Engine);
//    else if ( randomEngineName == "RanluxEngine")   G4Random::setTheEngine(new CLHEP::RanluxEngine);
//    else if ( randomEngineName == "RanshiEngine")   G4Random::setTheEngine(new CLHEP::RanshiEngine);
//    else if ( randomEngineName == "Hurd288Engine")  G4Random::setTheEngine(new CLHEP::Hurd288Engine);
//    else if ( randomEngineName == "TripleRand")     G4Random::setTheEngine(new CLHEP::TripleRand);
//    else {
//        cout << FATALERRORL << " Random engine >" << randomEngineName << "< not found. Exiting." << endl;
//        gexit(EC__RANDOMENGINENOTFOUND);
//    }

//    CLHEP::MixMaxRng defaultEngine(seed);
//    G4Random::setTheEngine( &defaultEngine );

    G4Random::setTheSeed(seed);


}
