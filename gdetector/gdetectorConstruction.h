#ifndef GDETECTORCONSTRUCTION_H
#define GDETECTORCONSTRUCTION_H 1

// gemc
#include "gStateMessage.h"

// glibrary
#include "goptions.h"
#include "gdynamicdigitization.h"
#include "gworld.h"
#include "g4world.h"

// geant4
#include "G4VUserDetectorConstruction.hh"

class GDetectorConstruction : public G4VUserDetectorConstruction, public GStateMessage
{
public:
	// constructor and destructor.
	GDetectorConstruction(GOptions* opt, map<string, GDynamicDigitization*> *gDDGlobal);
	virtual ~GDetectorConstruction();
	
public:
	// virtual methods from G4VUserDetectorConstruction
	virtual G4VPhysicalVolume* Construct();
	virtual void ConstructSDandField();
	
//	map<string, string> getSensitiveVolumes() {
//		if(gsetup != nullptr)
//			return gsetup->getSensitiveVolumes();
//		else
//			return {};
//	}
	
private:
	GOptions *gopt    = nullptr;
	GWorld   *gworld  = nullptr;
	G4World  *g4world = nullptr;

	// the GSensitiveDetector is built before the digitization, so we need
	// a pointer to global digitization map, filled later, to pass to the local GSensitiveDetector
	map<string, GDynamicDigitization*> *gDynamicDigitizationMapGlobalInstance = nullptr;


	// loads digitization plugins
	// returns number of loaded plugins
	int fillgDynamicDigitizationMapGlobalInstance();

};


#endif
