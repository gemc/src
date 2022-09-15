#include "gEventAction.h"

// geant4
#include "G4EventManager.hh"


GEventAction::GEventAction(GOptions* gopt) : G4UserEventAction()
{
	elog = gopt->getInt("elog");
}

GEventAction::~GEventAction()
{
	// G4cout << GEVENTMESSAGEHEADER << " GEventAction destructor " << G4endl;
}

void GEventAction::BeginOfEventAction(const G4Event* event)
{
	// printEventStatsBegin(event);
}

void GEventAction::EndOfEventAction(const G4Event* event)
{
	// printEventStatsEnd(event);
}

