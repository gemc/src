// gemc
#include "gEventAction.h"
#include "gutsConventions.h"

// geant4
#include "G4Event.hh"

// c++
#include <string>

// logs event statistics
void GEventAction::event_print_stats(const G4Event* event, bool start)
{
	int eventID = event->GetEventID();
    string eventIDs = std::to_string(eventID);
    string header = GEVENTMESSAGEHEADER;

    if (eventID % elog == 0 ) {
        if (start) {
            gLogMessage(header + " Begin of GEventAction for event " +  eventIDs);
        } else {
            gLogMessage(header + " End of GEventAction for event " +  eventIDs);
        }
    }
}
