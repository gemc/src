// gemc
#include "gaction.h"
#include "event/gEventAction.h"
#include "generator/gPrimaryGeneratorAction.h"
#include "run/gRunAction.h"

// notice gopts is copied here so the move affects the local parameter copy
GAction::GAction(std::shared_ptr<GOptions> gopts,
				 std::shared_ptr<gdynamicdigitization::dRoutinesMap> digi_map) :
	GBase(gopts, GACTION_LOGGER),
	goptions(std::move(gopts)),
	digitization_routines_map(std::move(digi_map)) { }


// Master
void GAction::BuildForMaster() const {
	// Master-thread registration: only the run action is required.
	log->debug(NORMAL, FUNCTION_NAME);

	SetUserAction(new GRunAction(goptions, digitization_routines_map));
}

// Workers
void GAction::Build() const {
	// Worker-thread registration: primary generator, run action, then event action.
	const auto thread_id = G4Threading::G4GetThreadId();

	log->debug(NORMAL, FUNCTION_NAME, "thread id: " + std::to_string(thread_id));

	SetUserAction(new GPrimaryGeneratorAction(goptions));

	auto* run_action = new GRunAction(goptions, digitization_routines_map);
	SetUserAction(run_action);

	SetUserAction(new GEventAction(goptions, run_action));
}