#ifndef  GSENSITIVEDETECTOR_H
#define  GSENSITIVEDETECTOR_H 1

// c++
#include <set>

// gemc
#include "gStateMessage.h"

// geant4
#include "G4VSensitiveDetector.hh"

// glibrary
#include "goptions.h"
#include "gdynamicdigitization.h"

using GHitsCollection = G4THitsCollection<GHit> ;

// this is thread-local
class GSensitiveDetector : public G4VSensitiveDetector, public GStateMessage
{

public:
	GSensitiveDetector(string sdName, GOptions* goptions, map<string, GDynamicDigitization*> *gDDGlobal);

	~GSensitiveDetector() {}

	// G4VSensitiveDetector geant4 methods to be overloaded
	virtual void Initialize(G4HCofThisEvent* g4hc);                            // Beginning of sensitive Hit
	virtual G4bool ProcessHits(G4Step* thisStep, G4TouchableHistory* g4th);    // Process Step, add new hit to gHitsCollection or new step to a ghit
	virtual void EndOfEvent(G4HCofThisEvent* g4HitCollection);                 // End of sensitive Hit


private:
	int verbosity;
	bool recordZeroEdep;

	// the GSensitiveDetector is built before the digitization, so we need
	// a pointer to global digitization map so we can pick gDigiLocal at initialization
	// it will be loaded later with the plugins
	map<string, GDynamicDigitization*> *gDynamicDigitizationMapGlobalInstance ;

	// gDigiLocal is thread local, picked form gDigitizationGlobal
	GDynamicDigitization *gDynamicDigitizationLocalInstance ;

	// defines what information to be stored in the hit
	// set at construction, so we do not spend time retrieving it
	HitBitSet gHitBitSet;

	// map of touchables. one entry per gvolume
	// used to retrieve the touchable from the geant4 volume in which the step occur during processHit
	// the map is populated (registered) at detector construction (GDetectorConstruction::ConstructSDandField)
	// using the public function registerGVolumeTouchable
	map<string, GTouchable*> gTouchableMap;

	// retrieve GTouchable from map
	// gTouchableMap[vname] is guaranteed to exist because vname is sensitive
	inline GTouchable* getGTouchable(const G4Step* thisStep){
		string vname = thisStep->GetPreStepPoint()->GetTouchable()->GetVolume()->GetName();
		return gTouchableMap[vname];
	}

	// GTouchable set, reset each event,
	// used to decide if this is a new hit or not
	std::set<GTouchable> touchableSet;

	// checking if it is present in the set. If not, add it.
	bool isThisANewTouchable(const GTouchable* thisTouchable);

	// GHit collection is G4THitsCollection<GHit>
	// GHit is a G4VHit. Its thread memory management come with geant4
	// The collection of pointers is used in G4GRun, passed to the digitizer plugins
	GHitsCollection *gHitsCollection;

	// retrieve hit with existing gtouchable
	GHit *getHitInHitCollectionUsingTouchable(const GTouchable* gtouchable);

	// decides if the hit should be processed or not
	// can be expanded with gDynamicDigitizationLocalInstance decision or other parameters
	bool decisionToSkipHit(double energy);


public:

	// register GTouchable in gTouchableMap
	// used in GDetectorConstruction::ConstructSDandField()
	void registerGVolumeTouchable(string name, GTouchable* gt);



};

#endif

