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
GStateMessage(gopts, "GRunAction", "verbosity"),  // GRunAction derived
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

	return new GRun(goptions, gDigitizationGlobalMap, gstreamerFactoryMap);
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
	
	if(IsMaster()) {
		string logMessage =  "EndOfRunAction Master for run id " + to_string(aRun->GetRunID()) + " in g4thread " + to_string(G4Threading::G4GetThreadId());
		logSummary(logMessage);
		logSummary("Total number of events this run: " + to_string(theRun->GetNumberOfEvent()));
	} else {
		string logMessage =  "EndOfRunAction Local for run id " + to_string(aRun->GetRunID()) + " in g4thread " + to_string(G4Threading::G4GetThreadId());
		logSummary(logMessage);
		logSummary("Total number of events this thread: " + to_string(theRun->GetNumberOfEvent()));
	}
}
