// geant4
#include "G4Event.hh"

// gemc
#include "gRun.h"


// Construct the thread-local run object and retain access to the shared
// digitization-routine map for the lifetime of the run.
GRun::GRun(std::shared_ptr<GOptions> gopt,
		   std::shared_ptr<gdynamicdigitization::dRoutinesMap> digi_map) :
	GBase(gopt, GRUN_LOGGER),
	digitization_routines_map(std::move(digi_map)) {
	const auto desc = "GRun " + std::to_string(G4Threading::G4GetThreadId());
	log->debug(CONSTRUCTOR, FUNCTION_NAME, desc);
}