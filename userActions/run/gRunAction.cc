// gemc
#include "gRunAction.h"
#include "gRun.h"

// geant4
#include "globals.hh"
#include "G4Threading.hh"
#include "G4MTRunManager.hh"


// c++
using namespace std;

// Constructor for workers
GRunAction::	GRunAction(GOptions* gopts, map<string, GDynamicDigitization*> *gDDGlobal, map<string, GStreamer*> *streamerFactoryMap) :
G4UserRunAction(), // G4UserRunAction derived
GStateMessage(gopts, "GRunAction", "grunv"),  // GRunAction derived
goptions(gopts),
gDigitizationGlobalMap(gDDGlobal),
gstreamerFactoryMap(streamerFactoryMap)
{
	logSummary("Instantiating GRunAction ");
}


// Destructor
GRunAction::~GRunAction()
{
}

// TODO: this is not local?
G4Run* GRunAction::GenerateRun()
{
	logSummary("GRunAction GenerateRun ");

	return new GRun(goptions, gDigitizationGlobalMap);
}

// executed after BeamOn
void GRunAction::BeginOfRunAction(const G4Run* aRun)
{

	if(IsMaster()) {
		string logMessage =  "BeginOfRunAction Master for run id " + to_string(aRun->GetRunID()) + " in g4thread " + to_string(G4Threading::G4GetThreadId());
		logSummary(logMessage);
	} else {
		string logMessage =  "BeginOfRunAction Local for run id " + to_string(aRun->GetRunID()) + " in g4thread " + to_string(G4Threading::G4GetThreadId());
		logSummary(logMessage);
	}
	
	
}

void GRunAction::EndOfRunAction(const G4Run* aRun)
{
	const GRun* theRun = static_cast<const GRun*>(aRun);

//	"GRun: local run data size " << localRun->runData.size() << "  global size: " << runData.size() << G4endl

	if(IsMaster()) {
		string logMessage =  "EndOfRunAction Master, run " + to_string(aRun->GetRunID()) + " in g4thread " + to_string(G4Threading::G4GetThreadId());
		logMessage += ", data size:  "  + to_string(theRun->getRunData().size());
		logSummary(logMessage);
		logSummary("Total number of events this run: " + to_string(theRun->GetNumberOfEvent()));


		


	} else {
		string logMessage =  "EndOfRunAction Local, run " + to_string(aRun->GetRunID()) + " in g4thread " + to_string(G4Threading::G4GetThreadId());
		logMessage += ", data size:  "  + to_string(theRun->getRunData().size());
		logSummary(logMessage);
		logSummary("Total number of events this thread: " + to_string(theRun->GetNumberOfEvent()));
	}


	//
	//	// output data to all available plugins
	//	for(auto gmf: (*gmediaFactory)) {
	//		// protecting against DL failure
	//		if(gmf.second != nullptr) {
	//			gmf.second->publishData(localRun->runData);
	//		}
	//	}




}
