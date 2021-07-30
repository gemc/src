#ifndef GEVENTACTION_H
#define GEVENTACTION_H 1

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
    
private:
    // logs event statistics
    void printEventStatsBegin(const G4Event* event);
    void printEventStatsEnd(const G4Event* event);

};


#endif
