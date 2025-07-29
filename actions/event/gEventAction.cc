#include "gEventAction.h"

// geant4
#include "G4EventManager.hh"


GEventAction::GEventAction(std::shared_ptr<GOptions> gopt) :
	G4UserEventAction(),
	log(std::make_shared<GLogger>(gopt, EVENTACTION_LOGGER, "GEventAction")) { log->debug(CONSTRUCTOR, FUNCTION_NAME); }

GEventAction::~GEventAction() { log->debug(DESTRUCTOR, FUNCTION_NAME); }

void GEventAction::BeginOfEventAction([[maybe_unused]] const G4Event* event) { log->info(2, "Begin of event"); }

void GEventAction::EndOfEventAction([[maybe_unused]] const G4Event* event) { log->info(2, "End of event"); }
