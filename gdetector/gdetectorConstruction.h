#ifndef GDETECTORCONSTRUCTION_H
#define GDETECTORCONSTRUCTION_H 1

// geant4
#include "G4VUserDetectorConstruction.hh"

// gemc
#include "gStateMessage.h"
#include "gsd.h"
#include "goptions.h"
#include "gdynamicdigitization.h"
#include "gworld.h"
#include "g4world.h"
#include "gmagneto.h"

class GDetectorConstruction : public G4VUserDetectorConstruction, public GStateMessage {
public:
    // constructor and destructor.
    GDetectorConstruction(GOptions *gopts, map<string, GDynamicDigitization *> *gDDGlobal);

    virtual ~GDetectorConstruction();

public:
    // virtual methods from G4VUserDetectorConstruction
    virtual G4VPhysicalVolume *Construct();

    virtual void ConstructSDandField();

    inline vector <string> getDigitizationNamesList() const { return gworld->getSensitiveDetectorsList(); }

    bool is_empty() const { return g4world->is_empty() ; }

	int get_number_of_volumes() const {
		return gworld->get_number_of_volumes();
	}

	int get_number_of_g4_volumes() const {
		return g4world->get_number_of_volumes();
	}

private:
    GOptions *gopt = nullptr;
    GWorld *gworld = nullptr;
    G4World *g4world = nullptr;

    // the GSensitiveDetector is in gemc.cc because it is also passed to the eventDispenser
    // using the global instance of the digitization map
    map<string, GDynamicDigitization *> *gDynamicDigitizationMapGlobalInstance;

    // contains fields and field managers
    static G4ThreadLocal GMagneto *gmagneto;

};


#endif
