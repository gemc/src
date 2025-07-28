#pragma once

// geant4
#include "G4VUserActionInitialization.hh"

// gemc
#include "goptions.h"
#include "glogger.h"
#include "gdynamicdigitization.h"
#include "gstreamer.h"

// G4VUserActionInitialization is a newly introduced class for the user to instantiate
// user action classes (both mandatory and optional).
//
// All the user action classes are thread-local and instantiated only for worker treads,
// with the only exception of UserRunAction, which could be instantiated also for the master thread.
// All user actions must be registered through SetUserAction() protected method defined in the
// G4VUserActionInitialization base class.
//
// G4VUserActionInitialization has two virtual method to be implemented:
// - Build() should be used for defining user action classes for worker threads
//   as well as for the sequential mode.
// - BuildForMaster() should be used for defining only the UserRunAction for the master thread.


class GActionInitialization : public G4VUserActionInitialization {
public:
	GActionInitialization(std::shared_ptr<GOptions> gopt, gdynamicdigitization::dRoutinesMap digi_map);

	virtual ~GActionInitialization();

	virtual void Build() const;

	virtual void BuildForMaster() const;

private:
	std::shared_ptr<goptions> goptions; // keeping option pointer to construct the other actions
	std::shared_ptr<GLogger>  log;

	// digitization map, loaded in main(), passed here
	gdynamicdigitization::dRoutinesMap digitization_routines_map;
	std::shared_ptr<const dRoutinesMap>

};
