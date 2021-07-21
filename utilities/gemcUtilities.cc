// Qt
#include <QtWidgets>

// geant4 headers
#include "G4Threading.hh"

// c++
#include <iostream>
using namespace std;

// glibrary
#include "goptions.h"

// conventions, utilities, options definitions
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
		cout << GEMCLOGMSGITEM << " G4MTRunManager: using " << useThreads << " threads out of "  << allThreads << " available."  << endl;
	}
	
	return useThreads;
}
