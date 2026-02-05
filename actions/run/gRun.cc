// geant4
#include "G4Event.hh"

// gemc
#include "gRun.h"


// Constructor: initializes the thread-local run object and stores shared services.
GRun::GRun(std::shared_ptr<GOptions> gopt, std::shared_ptr<gdynamicdigitization::dRoutinesMap> digi_map) :
	GBase(gopt, GRUN_LOGGER),
	digitization_routines_map(digi_map) {
	auto desc = "GRun " + std::to_string(G4Threading::G4GetThreadId());
	log->debug(CONSTRUCTOR, FUNCTION_NAME, desc);
}
