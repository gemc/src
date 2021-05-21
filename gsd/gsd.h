#ifndef  GSENSITIVEDETECTOR_H
#define  GSENSITIVEDETECTOR_H 1



// gemc
#include "gStateMessage.h"

// geant4
#include "G4VSensitiveDetector.hh"

// glibrary
#include "goptions.h"
#include "gdynamicdigitization.h"
#include "gEventData.h"


// this is thread-local
class GSensitiveDetector : public G4VSensitiveDetector, public GStateMessage
{

public:
	GSensitiveDetector(string sdName, GOptions* goptions, map<string, GDynamicDigitization*> *gDDGlobal);

	// geant4 methods
	virtual void Initialize(G4HCofThisEvent* g4hc);                            // Beginning of sensitive Hit
	virtual G4bool ProcessHits(G4Step* thisStep, G4TouchableHistory* g4th);    // Process Step
	virtual void EndOfEvent(G4HCofThisEvent* g4HitCollection);                            // End of sensitive Hit


private:
	int verbosity;

	// the GSensitiveDetector is built before the digitization, so we need
	// a pointer to global digitization map so we can pick gDigiLocal at initialization
	// it will be loaded later with the plugins
	map<string, GDynamicDigitization*> *gDynamicDigitizationMapGlobalInstance;

	// gDigiLocal is thread local, picked form gDigitizationGlobal
	GDynamicDigitization *gDynamicDigitizationLocalInstance;

	// contains the data collection in each event
	GEventData *eventData;


	// map of touchable associated with each volume
	// used to retrieve the touchable from the volume during processHit.
	// the map is populated (registered) at detector construction with the volume gtouchable
	// register function: registerGVolumeTouchable
	map<string, GTouchable*> gTouchableMap;

public:

	// register GTouchable
	// used in GDetectorConstruction::ConstructSDandField()
	void registerGVolumeTouchable(string name, GTouchable* gt);



};

#endif

