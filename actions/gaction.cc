// gemc
#include "gaction.h"
#include "event/gEventAction.h"
#include "generator/gPrimaryGeneratorAction.h"
#include "run/gRunAction.h"


GAction::GAction(std::shared_ptr<GOptions> gopts, std::shared_ptr<gdynamicdigitization::dRoutinesMap> digi_map) :
	GBase(gopts, GACTION_LOGGER),
	goptions(gopts),
	digitization_routines_map(digi_map) {  }


void GAction::BuildForMaster() const {
	// Master-thread registration: register the run action only.
	log->debug(NORMAL, FUNCTION_NAME);

	SetUserAction(new GRunAction(goptions, digitization_routines_map));
}


void GAction::Build() const {
	// Worker-thread registration: primary generator, run action, then event action.
	auto thread_id = G4Threading::G4GetThreadId();

	log->debug(NORMAL, FUNCTION_NAME, "thread id: " + std::to_string(thread_id));

	SetUserAction(new GPrimaryGeneratorAction(goptions));

	auto* run_action = new GRunAction(goptions, digitization_routines_map);
	SetUserAction(run_action);

	SetUserAction(new GEventAction(goptions, run_action));
}
