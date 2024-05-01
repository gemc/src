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
#include "gfield.h"


class GDetectorConstruction : public G4VUserDetectorConstruction, public GStateMessage
{
public:
	// constructor and destructor.
	GDetectorConstruction(GOptions* gopts, map<string, GDynamicDigitization*> *gDDGlobal);
	virtual ~GDetectorConstruction();
	
public:
	// virtual methods from G4VUserDetectorConstruction
	virtual G4VPhysicalVolume* Construct();
	virtual void ConstructSDandField();
	inline vector<string> getDigitizationNamesList() const { return gworld->getSensitiveDetectorsList();}
	
private:
	GOptions *gopt    = nullptr;
	GWorld   *gworld  = nullptr;
	G4World  *g4world = nullptr;

	// the GSensitiveDetector is in gemc.cc because it is also passed to the eventDispenser
	// using the global instance of the digitization map
	map<string, GDynamicDigitization*> *gDynamicDigitizationMapGlobalInstance;

    // map of GField pointers
    map<string, GField*> gFieldMap;

};


#endif
