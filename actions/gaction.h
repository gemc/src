#pragma once

// geant4
#include "G4VUserActionInitialization.hh"

// gemc
#include "glogger.h"
#include "gdynamicdigitization.h"
#include "gstreamer.h"

// the other actions, so we can define the options all at once
#include "event/gEventAction.h"
#include "run/gRunAction.h"
#include "run/gRun.h"
#include "generator/gPrimaryGeneratorAction.h"

// G4VUserActionInitialization is a newly introduced class for the user to instantiate
// user action classes (both mandatory and optional).
//
// All the user action classes are thread-local and instantiated only for worker treads,
// with the only exception of UserRunAction, which could be instantiated also for the master thread.
// All user actions must be registered through SetUserAction() protected method defined in the
// G4VUserActionInitialization base class.
//
// G4VUserActionInitialization has two virtual methods to be implemented:
// - Build() should be used for defining user action classes for worker threads
//   as well as for the sequential mode.
// - BuildForMaster() should be used for defining only the UserRunAction for the master thread.

constexpr const char* GACTION_LOGGER = "gaction";

namespace gaction {
inline GOptions defineOptions() {
	auto goptions = GOptions(GACTION_LOGGER);
	goptions += geventaction::defineOptions();
	goptions += grunaction::defineOptions();
	goptions += gprimaryaction::defineOptions();
	goptions += grun::defineOptions();
	return goptions;
}
}


class GAction : public G4VUserActionInitialization {
public:
	GAction(std::shared_ptr<GOptions> gopts, std::shared_ptr<gdynamicdigitization::dRoutinesMap> digi_map);

	~GAction() override;

	void Build() const override;

	void BuildForMaster() const override;

private:
	std::shared_ptr<GOptions> goptions; // keeping the goption pointer to construct the other actions
	std::shared_ptr<GLogger>  log;

	// digitization map, populated in ConstructSDandField
	std::shared_ptr<gdynamicdigitization::dRoutinesMap> digitization_routines_map;

};
