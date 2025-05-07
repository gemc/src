#pragma once

// geant4
#include "G4VUserActionInitialization.hh"

// gemc
#include "goptions.h"
#include "gdynamicdigitization.h"
#include "gstreamer.h"
#include "gStateMessage.h"

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


class GActionInitialization : public G4VUserActionInitialization, public GStateMessage {
public:
    GActionInitialization(GOptions *gopt, map<string, GDynamicDigitization *> *gDDGlobal);

    virtual ~GActionInitialization();

    virtual void Build() const;

    virtual void BuildForMaster() const;

private:
    GOptions *goptions;

    // digitization map, loaded in main(), passed here
    map<string, GDynamicDigitization *> *gDigitizationGlobalMap;

    // output factories map, loaded in the constructor
    map<string, GStreamer *> *gstreamerFactoryMap;

};



