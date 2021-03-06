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

	~GSensitiveDetector() {
		delete gHitsCollection;
		delete gDynamicDigitizationLocalInstance;
		delete gDynamicDigitizationMapGlobalInstance;
	}

	// geant4 methods
	virtual void Initialize(G4HCofThisEvent* g4hc);                            // Beginning of sensitive Hit
	virtual G4bool ProcessHits(G4Step* thisStep, G4TouchableHistory* g4th);    // Process Step, add new hit to gHitsCollection or new step to a ghit
	virtual void EndOfEvent(G4HCofThisEvent* g4HitCollection);                 // End of sensitive Hit


private:
	int verbosity;

	// the GSensitiveDetector is built before the digitization, so we need
	// a pointer to global digitization map so we can pick gDigiLocal at initialization
	// it will be loaded later with the plugins
	map<string, GDynamicDigitization*> *gDynamicDigitizationMapGlobalInstance = nullptr;

	// gDigiLocal is thread local, picked form gDigitizationGlobal
	GDynamicDigitization *gDynamicDigitizationLocalInstance = nullptr;

	// defines what information to be stored in the hit
	// set at construction, so we do not spend time retrieving it
	HitBitSet gHitBitSet;

	// map of touchable associated with each volume
	// used to retrieve the touchable from the volume in which the step occur during processHit.
	// the map is populated (registered) at detector construction with the volume gtouchable
	// register function: registerGVolumeTouchable
	map<string, GTouchable*> gTouchableMap;

	// retrieve touchable from map
	// needs geant4Touchable to get the volume name
	// vname is guaranteed to exist because it's sensitive
	inline GTouchable* getGTouchable(const G4Step* thisStep) {
		string vname = thisStep->GetPreStepPoint()->GetTouchable()->GetVolume()->GetName();
		return gTouchableMap[vname];
	}

	// GTouchable set, reset each event,
	// used to decide if this is a new hit or not
	std::set<GTouchable*> touchableSet;

	// by checking if it is present in the set. If not, add it.
	inline bool isThisANewTouchable(GTouchable* thisTouchable)
	{
		// not found. Insert it and return false
		if(touchableSet.find(thisTouchable) == touchableSet.end()) {
			touchableSet.insert(thisTouchable);
			return true;
		}

		return false;
	}

	// GHit collection is G4THitsCollection<GHit>
	// GHit is a G4VHit. Its thread memory management come with geant4
	// The collection of pointers is used in G4GRun, passed to the digitizer plugins
	GHitsCollection *gHitsCollection = nullptr;

	// retrieve hit with existing gtouchable
	GHit *getHitInHitCollectionUsingTouchable(GTouchable* gtouchable);

	// decides if the hit should be processed or not
	// can be expanded with gDynamicDigitizationLocalInstance decision or other parameters
	bool decisionToSkipHit(double energy);


public:

	// register GTouchable
	// used in GDetectorConstruction::ConstructSDandField()
	void registerGVolumeTouchable(string name, GTouchable* gt);



};

#endif

