// geant4
#include "G4Event.hh"

// gemc
#include "gRun.h"


// Constructor
GRun::GRun(std::shared_ptr<GOptions> gopt, std::shared_ptr<gdynamicdigitization::dRoutinesMap> digi_map) :
	digitization_routines_map(digi_map) {
	auto desc = "GRun " + std::to_string(G4Threading::G4GetThreadId());
	log       = std::make_shared<GLogger>(gopt, GRUN_LOGGER, desc);

	log->debug(CONSTRUCTOR, FUNCTION_NAME);
}

// Destructor
GRun::~GRun() {
	log->debug(DESTRUCTOR, FUNCTION_NAME);
}

