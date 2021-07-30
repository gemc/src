// gemc
#include "gEventAction.h"

// geant4
#include "G4Event.hh"


// logs event statistics
void GEventAction::printEventStatsBegin(const G4Event* event)
{
    int eventID = event->GetEventID();
    
    G4cout << GEVENTMESSAGEHEADER << " Begin of GEventAction for event " << eventID << G4endl;
}

// logs event statistics
void GEventAction::printEventStatsEnd(const G4Event* event)
{
    int eventID = event->GetEventID();
    
    G4cout << GEVENTMESSAGEHEADER << " End of GEventAction for event " << eventID << G4endl;

}
