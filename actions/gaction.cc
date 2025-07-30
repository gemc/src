// gemc
#include "gaction.h"
#include "event/gEventAction.h"
#include "generator/gPrimaryGeneratorAction.h"
#include "run/gRunAction.h"


GAction::GAction(std::shared_ptr<GOptions> gopts, std::shared_ptr<gdynamicdigitization::dRoutinesMap> digi_map):
	goptions(gopts),
	log(std::make_shared<GLogger>(gopts, GACTION_LOGGER, "GAction")),
	digitization_routines_map(digi_map) {
	log->debug(CONSTRUCTOR, FUNCTION_NAME);
}


GAction::~GAction() { log->debug(DESTRUCTOR, FUNCTION_NAME); }


void GAction::BuildForMaster() const {
	log->debug(NORMAL, FUNCTION_NAME);

	SetUserAction(new GRunAction(goptions, digitization_routines_map));
}


void GAction::Build() const {
	auto thread_id =  G4Threading::G4GetThreadId();

	log->debug(NORMAL, FUNCTION_NAME, "thread id: " + std::to_string(thread_id));

	SetUserAction(new GPrimaryGeneratorAction(goptions));
	SetUserAction(new GRunAction(goptions, digitization_routines_map));
	SetUserAction(new GEventAction(goptions));
}
