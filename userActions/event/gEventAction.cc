#include "gEventAction.h"

// geant4
#include "G4EventManager.hh"


GEventAction::GEventAction(GOptions* gopt) : G4UserEventAction()
{
    elog = gopt->getVerbosityFor("em_log");
}

GEventAction::~GEventAction()
{
	// G4cout << GEVENTMESSAGEHEADER << " GEventAction destructor " << G4endl;
}

void GEventAction::BeginOfEventAction([[maybe_unused]] const G4Event* event)
{
	// printEventStatsBegin(event);
}

void GEventAction::EndOfEventAction([[maybe_unused]] const G4Event* event)
{
	// printEventStatsEnd(event);
}
