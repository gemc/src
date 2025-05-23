#pragma once

// geant4
#include "G4Run.hh"

// gemc
#include "gStateMessage.h"

// glibrary
#include "goptions.h"
#include "gdynamicdigitization.h"
#include "event/gEventDataCollection.h"

// In Geant4 a run consists of a sequence of events and starts with BeamOn() method of G4RunManager.
// A Geant4 run is represented by a G4Run class object.
// G4GRun is created by G4MTRunManager, initialized in gemc.cc
// The pointers to the digitization and streamer are kept to be passed along RecordEvent (digitized event) and Merge (streams it out)
class GRun : public G4Run
{
public:
	GRun(GOptions* gopt, map<string, GDynamicDigitization*> *gDDGlobal);
	virtual ~GRun();
	virtual void RecordEvent(const G4Event*);
	virtual void Merge(const G4Run*);
	
private:
	int verbosity;
	int elog;
	
	// digitization map, loaded in main(), passed here through GActionInitialization
	map<string, GDynamicDigitization*> *gDigitizationGlobalMap;
	
	// vector of events data in the run (local thread, merged in GRun::Merge in the global thread)
	vector<GEventDataCollection*> runData;

private:
	GDynamicDigitization *getDigitizationForHitCollection(string name);

public:
	inline vector<GEventDataCollection*> getRunData() const {return runData;}

};
