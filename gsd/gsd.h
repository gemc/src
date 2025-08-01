#pragma once

// geant4
#include "G4VSensitiveDetector.hh"

// gemc
#include "goptions.h"
#include "gdynamicdigitization.h"
#include "glogger.h"

constexpr int         ERR_DYNAMICPLUGINNOTFOUND   = 2001;
constexpr int         ERR_HITNOTFOUNDINCOLLECTION = 2002;
constexpr int         ERR_NOCOLLECTION            = 2003;
constexpr const char* GSENSITIVE_LOGGER            = "gsd";

namespace gsensitivedetector {
inline GOptions defineOptions() { return GOptions(GSENSITIVE_LOGGER); }
}


using GHitsCollection = G4THitsCollection<GHit>;

// this is thread-local
class GSensitiveDetector : public G4VSensitiveDetector {

public:
	GSensitiveDetector(const std::string&                    sdName,
	                   const std::shared_ptr<GOptions>&      goptions);

//	~GSensitiveDetector() override { log->debug(DESTRUCTOR, "GSensitiveDetector"); }

	// G4VSensitiveDetector geant4 methods to be overloaded
	void   Initialize(G4HCofThisEvent* g4hc) override;                       // Beginning of sensitive Hit
	G4bool ProcessHits(G4Step* thisStep, G4TouchableHistory* g4th) override; // Process Step, add new hit to gHitsCollection or new step to a ghit
	void   EndOfEvent(G4HCofThisEvent* g4HitCollection) override;            // End of sensitive Hit

	void assign_digi_routine(std::shared_ptr<GDynamicDigitization> digi_routine) {
		digitization_routine = digi_routine;
	}

private:
	std::shared_ptr<GLogger> log;

	// thread local - digitization for this sensitive detector
	std::shared_ptr<GDynamicDigitization> digitization_routine;

	// defines what information to be stored in the hit
	// set at construction, so we do not spend time retrieving it
	HitBitSet gHitBitSet;

	// map of touchables. one entry per gvolume
	// used to retrieve the touchable from the geant4 volume in which the step occurs during processHit
	std::map<std::string, std::shared_ptr<GTouchable>> gTouchableMap;

	// retrieve GTouchable from map
	// gTouchableMap[vname] is guaranteed to exist because
	// the map is populated (registered) at detector construction (GDetectorConstruction::ConstructSDandField)
	// using the public function registerGVolumeTouchable
	inline std::shared_ptr<GTouchable> getGTouchable(const G4Step* thisStep) {
		std::string vname = thisStep->GetPreStepPoint()->GetTouchable()->GetVolume()->GetName();

		auto it = gTouchableMap.find(vname);
		if (it != gTouchableMap.end()) { return it->second; }
		// if not found, log an error
		log->error(ERR_DYNAMICPLUGINNOTFOUND, "GTouchable for volume " + vname + " not found in gTouchableMap");
	}

	// GTouchable vector, reset each event,
	// used to decide if this is a new hit or not
	std::vector<GTouchable> touchableVector;

	// checking if it is present in the map. If not, add it
	bool isThisANewTouchable(const std::shared_ptr<GTouchable>& thisTouchable);

	// GHitsCollection is G4THitsCollection<GHit>
	// GHit is a G4VHit. Its thread memory management comes with geant4
	// The collection of pointers is used in G4GRun, passed to the digitizer plugins
	GHitsCollection* gHitsCollection;

	// retrieve hit with existing gtouchable
	GHit* getHitInHitCollectionUsingTouchable(const std::shared_ptr<GTouchable>& gtouchable);

public:
	// register GTouchable in gTouchableMap

	// used in GDetectorConstruction::ConstructSDandField()
	inline void registerGVolumeTouchable(const std::string& name, std::shared_ptr<GTouchable> gt) {
		log->info(2, "Registering touchable gvolume <" + name + "> with value: " + gt->getIdentityString());

		gTouchableMap[name] = gt; // store the GTouchable in the map, we own it now
	}


};
