// gemc
#include "gsd.h"
#include "gemcConventions.h"

// geant4
#include "G4SDManager.hh"

// this is thread-local
GSensitiveDetector::GSensitiveDetector(string sdName, GOptions* goptions, map<string, GDynamicDigitization*> *gDDGlobal) :
G4VSensitiveDetector(sdName),                                              // geant4 derived
GStateMessage(goptions, "GSensitiveDetector " + sdName, "gsensitivityv"),  // GStateMessage derived
gDynamicDigitizationMapGlobalInstance(gDDGlobal),
gHitsCollection(nullptr) {
	verbosity      = goptions->getInt("gsensitivityv");
	recordZeroEdep = goptions->getSwitch("recordZeroEdep");

	// protected, from G4VSensitiveDetector: it's a G4CollectionNameVector
	// not really used in gemc but it's no overhead to do it as in the examples
	string hitCollectionName = sdName + "__HitCollection";
	collectionName.insert(hitCollectionName);

	logSummary("Instantiating GSensitiveDetector " + sdName);
}


// thread local
// run at the beginning of the event
void GSensitiveDetector::Initialize(G4HCofThisEvent* g4hc)
{
	string sdName = GetName();
	logSummary("Initialize GSensitiveDetector " + sdName);

	// assinging thread local DynamicDigitization from the global map
	if(gDynamicDigitizationMapGlobalInstance->find(sdName) != gDynamicDigitizationMapGlobalInstance->end()) {
		gDynamicDigitizationLocalInstance = (*gDynamicDigitizationMapGlobalInstance)[sdName];
		gHitBitSet = gDynamicDigitizationLocalInstance->readoutSpecs->getHitBitSet();
	}

	// protecting against pluging loading failures
	if(!gDynamicDigitizationLocalInstance) {
		// gDynamicDigitizationLocalInstance not found, error: exit
		logError(" gDynamicDigitizationLocalInstance for " + sdName + " could not loaded.", EC__DYNAMICPLUGINNOTFOUND);
	}

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
}



// thread local
G4bool GSensitiveDetector::ProcessHits(G4Step* thisStep, G4TouchableHistory* g4th)
{
	// gDynamicDigitizationLocalInstance is guaranteed to exist in GSensitiveDetector::Initialize

	double depe = thisStep->GetTotalEnergyDeposit();

	// if there is a decision to skip this event, return
	if( decisionToSkipHit(depe) ) {
		return true;
	}
	
	// get the vector of GTouchables returned by gDynamicDigitizationLocalInstance
	// if not defined by the plugin, base class will return a vector with one element, the input
	vector<GTouchable*> thisStepProcessedTouchables = gDynamicDigitizationLocalInstance->processTouchable(getGTouchable(thisStep), thisStep);
	
	if ( verbosity >= GVERBOSITY_SUMMARY ) {
		G4cout << " ProcessHits starting loop with " <<thisStepProcessedTouchables.size() << " touchable(s) for step pointer" << thisStep ;
		G4cout << ", edep: " << depe << ", Hit collection size: " << gHitsCollection->GetSize()  << G4endl;
	}

	for(const auto thisGTouchable: thisStepProcessedTouchables) {
		
		if ( verbosity >= GVERBOSITY_SUMMARY ) {
			G4cout << " ProcessHits " << *thisGTouchable ;
		}
		// assign track id
		thisGTouchable->assignTrackId(thisStep->GetTrack()->GetTrackID());

		if(isThisANewTouchable(thisGTouchable)) {

			if ( verbosity >= GVERBOSITY_DETAILS ) {
				G4cout << " ProcessHits: found NEW hit with this touchable "  << G4endl << G4endl;
			}
			// new ghit, insert in gHitsCollection
			// the constructor takes care of the filling the hit step infos according to gHitBitSet
			gHitsCollection->insert(new GHit(thisGTouchable, thisStep, gHitBitSet));

		} else {
			// not a new touchable, must be in hit collection
			// retrieve hit from hit collection
			// add informations according to gHitBitSet
			GHit* existingHit = getHitInHitCollectionUsingTouchable(thisGTouchable);
						
			if ( verbosity >= GVERBOSITY_DETAILS ) {
				G4cout << " ProcessHits: adding hit infos for existingHit touchable "  << G4endl;
			}
			existingHit->addHitInfosForBitset(gHitBitSet, thisStep);
		}
	}


	return true;
}

// checking if it is present in the set. If not, add it.
bool GSensitiveDetector::isThisANewTouchable(const GTouchable* thisTouchable)
{
	GTouchable gtInst(*thisTouchable);
	
	// if not found insert and return true: it's a new
	auto gtPosition = find (touchableVector.begin(), touchableVector.end(), gtInst);
	
	if( gtPosition == touchableVector.end() ) {
		
		if ( verbosity >= GVERBOSITY_DETAILS ) {
			G4cout << " Not found in Gtouchable set containing: " << G4endl ;
			for ( size_t index = 0; auto gtinset: touchableVector ) {
				G4cout << "  - touchable index " << index << ":" << G4endl << gtinset ;
				index++;
			}
		}
		
		touchableVector.push_back(gtInst);
		return true;
	}
	
	return false;
}

GHit* GSensitiveDetector::getHitInHitCollectionUsingTouchable(const GTouchable* gtouchable) {

	for(unsigned int i=0; i<gHitsCollection->GetSize(); i++) {
		
		GHit* thisHit = (*gHitsCollection)[i];
		const GTouchable* thisHitGTouchable = thisHit->getGTouchable();
		
		if ( verbosity >= GVERBOSITY_DETAILS ) {
			G4cout << " getHitInHitCollectionUsingTouchable Hit n. " << i
			       << "  comparing thisHitGTouchable: " <<  *thisHitGTouchable << " with GTouchable "  << *gtouchable
					 << " comparison result: " << ( *thisHitGTouchable == *gtouchable ) << G4endl;
		}
		
		if( thisHitGTouchable == gtouchable ) {
			if ( verbosity >= GVERBOSITY_DETAILS ) {
				G4cout <<   " getHitInHitCollectionUsingTouchable Gtouchable found! "  << G4endl;
			}
			return thisHit;
		}
	}

	G4cout << " getHitInHitCollectionUsingTouchable  GTouchable "  << *gtouchable << " not found, error. Here is the looking loop: "  << G4endl;
	
	for(unsigned int i=0; i<gHitsCollection->GetSize(); i++) {
		
		GHit* thisHit = (*gHitsCollection)[i];
		const GTouchable* thisHitGTouchable = thisHit->getGTouchable();
		
		G4cout << " getHitInHitCollectionUsingTouchable Hit n. " << i
		<< "  comparing thisHitGTouchable: " <<  *thisHitGTouchable << " with GTouchable "  << *gtouchable
		<< " comparison result: " << ( *thisHitGTouchable == *gtouchable ) << G4endl;
		
	}

	
	// hit not found, error: exit
	logError("GHit not found in hit collection", EC__HITNOTFOUNDINCOLLECTION);

	return nullptr;
}


// thread local
void GSensitiveDetector::EndOfEvent(G4HCofThisEvent* g4hc)
{
	logSummary("EndOfEvent of GSensitiveDetector " + GetName());
}


// called in GDetectorConstruction::ConstructSDandField
void GSensitiveDetector::registerGVolumeTouchable(string name, GTouchable* gt)
{
	// not using log here as we're couting gt
	if(verbosity == GVERBOSITY_DETAILS) {
		G4cout << "Registering touchable gvolume <" << name << "> with  value: " << gt << G4endl;
	}
	gTouchableMap[name] = gt;
}


