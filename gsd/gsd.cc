// gemc
#include "gsd.h"

// geant4
#include "G4SDManager.hh"

// Thread-local sensitive detector instance.
// Constructor: initializes base logging, Geant4 SD name, and the hits collection name used for this detector.
GSensitiveDetector::GSensitiveDetector(const std::string&               sdName,
                                       const std::shared_ptr<GOptions>& goptions) :
	GBase(goptions, GSENSITIVE_LOGGER),
	G4VSensitiveDetector(sdName) {
	log->info(2, FUNCTION_NAME, " for " + sdName);

	// collectionName is a G4VSensitiveDetector G4CollectionNameVector.
	// GEMC uses its own handling of hits; this is set primarily for compatibility with Geant4 conventions/examples.
	std::string hitCollectionName = sdName + "__HitCollection";
	collectionName.insert(hitCollectionName);

	log->debug(CONSTRUCTOR, FUNCTION_NAME, " for " + sdName);
}


// Thread-local; called at the beginning of each event.
// Allocates and registers the per-event hits collection and resets per-event caches.
void GSensitiveDetector::Initialize(G4HCofThisEvent* g4hc) {
	std::string sdName = GetName();
	log->info(1, FUNCTION_NAME, sdName);

	// Hit content definition is read once per event from the digitization routine.
	gHitBitSet = digitization_routine->readoutSpecs->getHitBitSet();

	// Clearing touchableVector at the start of the event (per-event hit identity cache).
	touchableVector.clear();

	// Initializing gHitsCollection using the Geant4 G4THitsCollection constructor (expects detector and collection names).
	gHitsCollection = new GHitsCollection(sdName, collectionName[0]);

	// Add hits collection to the Geant4 event container.
	// hcID is assigned by Geant4 and can be retrieved later by collection name.
	auto hcID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
	g4hc->AddHitsCollection(hcID, gHitsCollection);

	log->info(2, "Added collection id ", hcID, " to G4HCofThisEvent");
}


// Thread-local; called for each step in sensitive volumes.
// Applies plugin filtering and touchable transformation, then creates new hits or updates existing hits.
G4bool GSensitiveDetector::ProcessHits(G4Step* thisStep, [[maybe_unused]] G4TouchableHistory* g4th) {
	// If there is a decision to skip this hit based on deposited energy, return immediately.
	double depe = thisStep->GetTotalEnergyDeposit();
	if (digitization_routine->decisionToSkipHit(depe)) { return true; }

	// The hits collection should have been created in Initialize().
	if (gHitsCollection == nullptr) { log->error(ERR_NOCOLLECTION, "No hit collection found"); }

	// Get the vector of processed touchables returned by the digitization routine.
	// If not overridden by the plugin, the base implementation typically returns a vector with the input touchable only.
	std::vector<std::shared_ptr<GTouchable>> thisStepProcessedTouchables =
		digitization_routine->processTouchable(getGTouchable(thisStep), thisStep);

	auto hcsize = gHitsCollection->GetSize();

	log->info(2, FUNCTION_NAME, " for ", GetName(),
	          " with ", std::to_string(thisStepProcessedTouchables.size()), " touchable(s), edep: ",
	          std::to_string(depe), ", Hit collection size: ", hcsize);

	// If a new touchable is created, create a new GHit.
	// Otherwise, retrieve the existing hit and append step information according to gHitBitSet.
	for (auto thisGTouchable : thisStepProcessedTouchables) {
		// Track id is attached to the touchable to keep hit identity consistent across updates.
		thisGTouchable->assignTrackId(thisStep->GetTrack()->GetTrackID());

		if (isThisANewTouchable(thisGTouchable)) {
			// New hit: constructor fills initial step information based on gHitBitSet.
			gHitsCollection->insert(new GHit(thisGTouchable, gHitBitSet, thisStep));
		}
		else {
			// Existing hit: locate it and add additional information from this step according to gHitBitSet.
			GHit* existingHit = getHitInHitCollectionUsingTouchable(thisGTouchable);
			if (existingHit != nullptr) {
				existingHit->addHitInfosForBitset(gHitBitSet, thisStep);
			}
		}
	}

	return true;
}


// Checks whether a touchable has already been seen in this event.
// If not present, it is appended to the per-event cache.
bool GSensitiveDetector::isThisANewTouchable(const std::shared_ptr<GTouchable>& thisTouchable) {
	log->info(2, "GSensitiveDetector::isThisANewTouchable for " + GetName(),
	          " with touchable: " + thisTouchable->getIdentityString());

	if (thisTouchable->exists_in_vector(touchableVector)) {
		log->info(2, " ❌ not a new GTouchable, it is found, retrieving hit...");
		return false; // Not a new touchable.
	}
	else {
		log->info(2, " ✅ yes, new GTouchable. Adding it to touchableVector.");
		touchableVector.push_back(*thisTouchable);
		return true; // It's a new touchable.
	}
}


// Linear search for the hit matching the provided touchable.
// The collection is expected to contain a matching entry when called from ProcessHits().
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

	log->error(ERR_HITNOTFOUNDINCOLLECTION,
	           "GHit not found in hit collection for touchable: " + gtouchable->getIdentityString(),
	           "in ", GetName());
}


// Thread-local end-of-event hook.
// At this stage, Geant4 owns the event hit container that references the hits collection.
void GSensitiveDetector::EndOfEvent([[maybe_unused]] G4HCofThisEvent* g4hc) {
	log->info(1, "GSensitiveDetector::EndOfEvent for " + GetName());
}
