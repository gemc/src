// gemc
#include "gsd.h"

// geant4
#include "G4SDManager.hh"

// this is thread-local
GSensitiveDetector::GSensitiveDetector(const std::string&               sdName,
                                       const std::shared_ptr<GOptions>& goptions) :
	G4VSensitiveDetector(sdName), // geant4 derived
	log(std::make_shared<GLogger>(goptions, GSENSITIVE_LOGGER, "gsd")) {
	log->info(2, FUNCTION_NAME, " for " + sdName);

	// collectionName is a G4VSensitiveDetector G4CollectionNameVector
	// not really used in gemc, using it here to mimic the examples
	std::string hitCollectionName = sdName + "__HitCollection";
	collectionName.insert(hitCollectionName);

	log->debug(CONSTRUCTOR, FUNCTION_NAME, " for " + sdName);
}


// thread local
// run at the beginning of the event
void GSensitiveDetector::Initialize(G4HCofThisEvent* g4hc) {
	std::string sdName = GetName();
	log->info(1, FUNCTION_NAME, sdName);

	gHitBitSet = digitization_routine->readoutSpecs->getHitBitSet();

	// clearing touchableVector at the start of the event
	touchableVector.clear();

	// initializing gHitsCollection collection using the geant4 G4THitsCollection constructor
	// this uses two arguments (not sure why)
	gHitsCollection = new GHitsCollection(sdName, collectionName[0]);

	// adding gHitsCollection to the G4HCofThisEvent
	// hcID is incrememnted by 1 every time we instantiate a new G4THitsCollection
	// it can then be retrieved at the end of the event
	auto hcID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
	g4hc->AddHitsCollection(hcID, gHitsCollection);

	log->info(2, "Added collection id ", hcID, " to G4HCofThisEvent");

}


// thread local
G4bool GSensitiveDetector::ProcessHits(G4Step* thisStep, [[maybe_unused]] G4TouchableHistory* g4th) {
	// if there is a decision to skip this event based on depe, return
	double depe = thisStep->GetTotalEnergyDeposit();
	if (digitization_routine->decisionToSkipHit(depe)) { return true; }
	if (gHitsCollection == nullptr) { log->error(ERR_NOCOLLECTION, "No hit collection found"); }

	// get the vector of GTouchables returned by gDynamicDigitization
	// if not defined by the plugin, base class will return a vector with one element (the input)
	std::vector<std::shared_ptr<GTouchable>> thisStepProcessedTouchables = digitization_routine->processTouchable(getGTouchable(thisStep), thisStep);

	auto hcsize = gHitsCollection->GetSize();

	log->info(2, FUNCTION_NAME, " for ", GetName(),
	          " with ", std::to_string(thisStepProcessedTouchables.size()), " touchable(s), edep: ",
	          std::to_string(depe), ", Hit collection size: ", hcsize);

	// if a new touchable is created, create a new GHit
	// otherwise, set the hit hitbitset
	for (auto thisGTouchable : thisStepProcessedTouchables) {
		// assign track id
		thisGTouchable->assignTrackId(thisStep->GetTrack()->GetTrackID());

		if (isThisANewTouchable(thisGTouchable)) {
			// new ghit, insert in gHitsCollection
			// the constructor takes care of the filling of the hit step according to gHitBitSet
			gHitsCollection->insert(new GHit(thisGTouchable, gHitBitSet, thisStep));
		}
		else {
			// not a new touchable, must be in the hit collection
			// retrieve it and add information according to gHitBitSet
			GHit* existingHit = getHitInHitCollectionUsingTouchable(thisGTouchable);
			existingHit->addHitInfosForBitset(gHitBitSet, thisStep);
		}
	}

	return true;
}

// checking if it is present in the map. If not, add it.
bool GSensitiveDetector::isThisANewTouchable(const std::shared_ptr<GTouchable>& thisTouchable) {
	log->info(2, "GSensitiveDetector::isThisANewTouchable for " + GetName(),
	          " with touchable: " + thisTouchable->getIdentityString());

	if (thisTouchable->exists_in_vector(touchableVector)) {
		log->info(2, " ❌ not a new GTouchable, it is found, retrieving hit...");
		return false; // not a new touchable
	}
	else {
		log->info(2, " ✅ yes, new GTouchable. Adding it to touchableVector.");
		touchableVector.push_back(*thisTouchable);
		return true; // it's a new touchable
	}
}


GHit* GSensitiveDetector::getHitInHitCollectionUsingTouchable(const std::shared_ptr<GTouchable>& gtouchable) {
	for (unsigned int i = 0; i < gHitsCollection->GetSize(); i++) {
		GHit*                              thisHit           = (*gHitsCollection)[i];
		const std::shared_ptr<GTouchable>& thisHitGTouchable = thisHit->getGTouchable();

		if (*gtouchable == *thisHitGTouchable) {
			log->info(2, "GSensitiveDetector::getHitInHitCollectionUsingTouchable for " + GetName() +
			             " found existing hit in collection for touchable: " + gtouchable->getIdentityString() +
			             " at index: " + std::to_string(i));
			return thisHit;
		}
	}

	log->error(ERR_HITNOTFOUNDINCOLLECTION, "GHit not found in hit collection for touchable: " + gtouchable->getIdentityString(), "in ", GetName());
}


// thread local
void GSensitiveDetector::EndOfEvent([[maybe_unused]] G4HCofThisEvent* g4hc) { log->info(1, "GSensitiveDetector::EndOfEvent for " + GetName()); }
