#include "gEventAction.h"

// geant4
#include "G4EventManager.hh"


GEventAction::GEventAction(GOptions *gopt) : G4UserEventAction() {
    elog = gopt->getScalarInt("event_module_log");
    verbosity = gopt->getVerbosityFor("event");
}

GEventAction::~GEventAction() {}

void GEventAction::BeginOfEventAction([[maybe_unused]] const G4Event *event) {
    if (verbosity >= GVERBOSITY_SUMMARY) event_print_stats(event, true);
}

void GEventAction::EndOfEventAction([[maybe_unused]] const G4Event *event) {
    if (verbosity >= GVERBOSITY_SUMMARY) event_print_stats(event, false);
}
