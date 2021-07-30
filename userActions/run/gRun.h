#ifndef GRUN_H
#define GRUN_H 1

// geant4
#include "G4Run.hh"

// gemc
#include "gStateMessage.h"

// glibrary
#include "goptions.h"
#include "gdynamicdigitization.h"
#include "event/gEventData.h"
#include "gstreamer.h"

// In Geant4 a run consists of a sequence of events and starts with BeamOn() method of G4RunManager.
// A run is represented by a G4Run class object.
// G4GRun is created by G4MTRunManager, initialized in gemc.cc
// The pointers to the digitization and streamer are kept to be passed along RecordEvent (digitized event) and Merge (streams it out)
class GRun : public G4Run, public GStateMessage
{
public:
	GRun(GOptions* gopt, map<string, GDynamicDigitization*> *gDDGlobal, map<string, GStreamer*> *gstrFactory);
	virtual ~GRun();
	virtual void RecordEvent(const G4Event*);
	virtual void Merge(const G4Run*);
	
private:
	// digitization map, loaded in main(), passed here
	map<string, GDynamicDigitization*> *gDigitizationGlobal;
	
	// output factories map, loaded in GActionInitialization constructor and passed here
	map<string, GStreamer*> *gstreamerFactory;

	// vector of events data in the local run
	vector<GEventData*> *runData;
	
private:
	GDynamicDigitization *getDigitizationForHitCollection(string name);

};


#endif
