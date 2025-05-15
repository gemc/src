#pragma once

// geant4
#include "G4UserEventAction.hh"

// glibrary
#include "goptions.h"

#define GEVENTMESSAGEHEADER  "✍︎"

// Local thread classes (g4cout)
class GEventAction : public G4UserEventAction
{
public:
	GEventAction(GOptions* gopt);
	virtual ~GEventAction();
	
	virtual void BeginOfEventAction(const G4Event* event);
	virtual void EndOfEventAction(const G4Event* event);

private:
    int elog; // print event log every elog events
    int verbosity;
    // todo: pass thread number from action here?
    // int thread_number;

private:
    // logs event statistics
    void event_print_stats(const G4Event* event, bool start);
};
