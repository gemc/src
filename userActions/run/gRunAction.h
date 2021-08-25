#ifndef GRUNACTION_H
#define GRUNACTION_H 1

// geant4
#include "G4UserRunAction.hh"

// gemc
#include "gStateMessage.h"

// glibrary
#include "goptions.h"
#include "gdynamicdigitization.h"
#include "gstreamer.h"
#include "frame/gFrameDataCollection.h"

class GRunAction : public G4UserRunAction, public GStateMessage
{
public:
	// constructor and destructor
	GRunAction(GOptions* gopts, map<string, GDynamicDigitization*> *gDDGlobal, map<string, GStreamer*> *streamerFactoryMap);
	virtual ~GRunAction();


private:
	// virtual method from G4UserRunAction.
	virtual G4Run* GenerateRun();
	virtual void BeginOfRunAction(const G4Run*);
	virtual void EndOfRunAction(const G4Run*);
	
	GOptions* goptions;
	
	// digitization map, loaded in main(), passed here
	map<string, GDynamicDigitization*> *gDigitizationGlobalMap;
	
	// output factories map, loaded in GActionInitialization constructor and passed here
	// the key is the sensitive detector name
	map<string, GStreamer*> *gstreamerFactoryMap;


	// vector of frame data in the run (local thread, merged in GRun::Merge in the global thread)
	vector<GFrameDataCollection*> frameRunData;
	double frameDuration = UNINITIALIZEDNUMBERQUANTITY; // frame length in nanoseconds
	double eventDuration = UNINITIALIZEDNUMBERQUANTITY; // event duration in nanoseconds
	int nthreads         = UNINITIALIZEDNUMBERQUANTITY;

	// determine the frame ID based on event number, eventDuration, frameDuration
	int const eventFrameID(int eventNumber, float timeAtElectronics) const;
	// decide whethere to write or not to stream the frame based on event number, eventDuration, frameDuration and number of threads
	// streaming the frame also deletes it from frameRunData
	bool writeFrameID(int eventNumber, int frameID);


};




#endif
