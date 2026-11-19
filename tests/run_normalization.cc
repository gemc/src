#include "gemc_options.h"
#include <gemc/actions/run/gRunAction.h>
#include <gemc/gtouchable/gtouchable.h>
#include <gemc/gphysics/gphysics.h>
#include "G4RunManager.hh"
#include "G4Run.hh"

#include <memory>

class FixedRunDigitizer : public GDynamicDigitization {
public:
	using GDynamicDigitization::GDynamicDigitization;
	bool defineReadoutSpecsImpl() override { return true; }
	CollectionMode collection_mode() const override { return CollectionMode::run; }
	std::vector<std::string> variables_to_normalizeImpl() override { return {"dose"}; }
};

int main(int argc, char* argv[]) {
	auto options = std::make_shared<GOptions>(argc, argv, gemc::defineOptions());
	auto manager = std::make_unique<G4RunManager>();
	auto physics = std::make_unique<GPhysics>(options);
	manager->SetUserInitialization(physics->getPhysList());
	auto routines = std::make_shared<gdynamicdigitization::dRoutinesMap>();
	(*routines)["fixed"] = std::make_shared<FixedRunDigitizer>(options);
	(*routines)["fixed"]->set_loggers(options);
	GRunAction action(options, routines);
	G4UserRunAction* callbacks = &action;
	G4Run run;
	run.SetRunID(11);
	run.SetNumberOfEventToBeProcessed(4);
	callbacks->BeginOfRunAction(&run);
	auto touchable = GTouchable::create(options);
	GHit hit(touchable);
	for (int event = 0; event < 4; ++event) {
		action.increment_run_events_processed();
		auto data = std::make_unique<GDigitizedData>(options, &hit);
		data->includeVariable("dose", 10.0);
		action.collect_event_data_collections("fixed", std::move(data));
	}
	callbacks->EndOfRunAction(&run);
	return 0;
}
