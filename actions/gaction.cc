// gemc
#include "gaction.h"
#include "event/gEventAction.h"
#include "generator/gPrimaryGeneratorAction.h"
#include "run/gRunAction.h"

// Construct the action initializer and keep shared services available for later
// action registration on worker and master execution contexts.
GAction::GAction(std::shared_ptr<GOptions>                           gopts,
				 std::shared_ptr<gdynamicdigitization::dRoutinesMap> digi_map) :
	GBase(gopts, GACTION_LOGGER),
	goptions(std::move(gopts)),
	digitization_routines_map(std::move(digi_map)) {

	log->debug(CONSTRUCTOR, FUNCTION_NAME);

}


// Register the master-thread actions.
// In GEMC, the master needs only the run action because it does not execute
// primary generation or per-event processing.
void GAction::BuildForMaster() const {
	log->debug(NORMAL, FUNCTION_NAME);

	SetUserAction(new GRunAction(goptions, digitization_routines_map));
}

// Register the worker-thread actions.
// Workers execute the full event lifecycle, so they need the primary generator,
// the run action, and the event action.
void GAction::Build() const {
	const auto thread_id = G4Threading::G4GetThreadId();

	log->debug(NORMAL, FUNCTION_NAME, "thread id: " + std::to_string(thread_id));

	// Primary generation is event-scoped and therefore worker-owned.
	SetUserAction(new GPrimaryGeneratorAction(goptions));

	// The run action is shared conceptually across the worker-thread lifecycle and
	// is passed to the event action so event processing can access run services.
	auto* run_action = new GRunAction(goptions, digitization_routines_map);
	SetUserAction(run_action);

	// The event action consumes the run action as a non-owning dependency.
	SetUserAction(new GEventAction(goptions, run_action));
}
