// gemc
#include "gRunAction.h"
#include "gRun.h"
#include "gemcUtilities.h"

// geant4
#include "globals.hh"
#include "G4Threading.hh"
#include "G4MTRunManager.hh"

// glibrary
#include "gstreamerConventions.h"
#include "gutilities.h"

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
	frameDuration = 64000;
	eventDuration = gutilities::getG4Number(gopts->getString("eventTimeSize"));
	nthreads      = getNumberOfThreads(gopts);

}


// Destructor
GRunAction::~GRunAction()
{
	// need to delete all data here
	
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

	if(IsMaster()) {
		string logMessage =  "EndOfRunAction Master, run " + to_string(aRun->GetRunID()) + " in g4thread " + to_string(G4Threading::G4GetThreadId());
		logMessage += ", data size:  "  + to_string(theRun->getRunData().size());
		logSummary(logMessage);
		logSummary("Total number of events this run: " + to_string(theRun->GetNumberOfEvent()));

		// looping over output factories

		for ( auto [factoryName, streamerFactory]: *gstreamerFactoryMap ) {
			logSummary( "Writing data using streamer factory >" + factoryName + "<") ;

			map<string, bool> streamReport = streamerFactory->publishRunData(goptions, theRun->getRunData());

			for ( auto [reportName, result]: streamReport ) {
				string resultString = result ? " success" : " failure";
				logSummary("Factory <" + factoryName + "> " + reportName + resultString);
			}

		}

		// looping over run data and filling frameRunData
		// need to remember last event number here
		for ( auto eventDataCollection: theRun->getRunData() ) {

			eventDataCollection->getHeader()->print();
			int evn = eventDataCollection->getEventNumber();

			for ( auto [detectorName, gdataCollection]: *eventDataCollection->getDataCollection()) {
				for ( auto hitDigitizedData: *gdataCollection->getDigitizedData() ) {
					float timeAtelectronic = hitDigitizedData->getflotObservable(TIMEATELECTRONICS);
					int frameID = eventFrameID(evn, timeAtelectronic);
				}

			}
		}

		// done with data, deleting it
		for ( auto* eventDataCollection: theRun->getRunData() )  { delete eventDataCollection; }


	} else {
		string logMessage =  "EndOfRunAction Local, run " + to_string(aRun->GetRunID()) + " in g4thread " + to_string(G4Threading::G4GetThreadId());
		logMessage += ", data size:  "  + to_string(theRun->getRunData().size());
		logSummary(logMessage);
		logSummary("Total number of events this thread: " + to_string(theRun->GetNumberOfEvent()));
	}

}


// determine the frame ID based on event number, eventDuration, frameDuration and number of threads
int const GRunAction::eventFrameID(int eventNumber, float timeAtElectronics) const {

	int absoluteHitTime = eventNumber*eventDuration + timeAtElectronics/CLHEP::ns;

	 cout << "eventNumber: " << eventNumber << ", absoluteHitTime: " << absoluteHitTime << ", frameDuration: " << frameDuration << " frameID: " << absoluteHitTime/frameDuration << endl;

	return absoluteHitTime/frameDuration;
}

bool GRunAction::writeFrameID(int eventNumber, int frameID) {
	int minEventNumber = eventNumber - 2*nthreads;

	int minEventTime = minEventNumber*eventDuration ;

	return false;
}
