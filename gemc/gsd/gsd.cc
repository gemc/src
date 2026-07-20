// gemc
#include "gsd.h"

// geant4
#include "G4SDManager.hh"

// Thread-local sensitive detector instance.
// Constructor: initializes base logging, Geant4 SD name, and the hits collection name used for this detector.
GSensitiveDetector::GSensitiveDetector(const std::string&               sdName,
                                       const std::shared_ptr<GOptions>& goptions) :
	GBase(goptions, GSENSITIVE_LOGGER),
	G4VSensitiveDetector(sdName),
	gHitsCollection(nullptr) {
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

	// Clearing the per-event hit-cell map (per-event hit identity cache).
	hitsByCellKey.clear();
	GHit::clearTrackVertexCache();

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

	if (digitization_routine->decisionToSkipHit(depe, thisStep)) { return true; }

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

	for (auto thisGTouchable : thisStepProcessedTouchables) {
		// Track id is attached to the touchable to keep hit identity consistent across updates.
		thisGTouchable->assignTrackId(thisStep->GetTrack()->GetTrackID());
		thisGTouchable->assignPId(thisStep->GetTrack()->GetDefinition()->GetPDGEncoding());

		// Create-or-update through the per-event hit-cell map: the key has the same
		// semantics as GTouchable::operator== (identity + type discriminator).
		auto [it, isNewCell] = hitsByCellKey.try_emplace(thisGTouchable->cellKey(), nullptr);
		if (isNewCell) {
			log->info(2, " ✅ new GTouchable for ", GetName(), ": ", thisGTouchable->getIdentityString());
			it->second = new GHit(thisGTouchable, thisStep);
			gHitsCollection->insert(it->second);
		}
		else {
			log->info(2, " ❌ existing GTouchable for ", GetName(), ": ", thisGTouchable->getIdentityString());
			it->second->addHitInfos(thisStep);
		}
	}

	return true;
}


// Thread-local end-of-event hook.
// At this stage, Geant4 owns the event hit container that references the hits collection.
void GSensitiveDetector::EndOfEvent([[maybe_unused]] G4HCofThisEvent* g4hc) {
	log->info(1, "GSensitiveDetector::EndOfEvent for " + GetName());
}
