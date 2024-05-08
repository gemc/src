// gemc
#include "gsd.h"
#include "gemcConventions.h"

// geant4
#include "G4SDManager.hh"

// this is thread-local
GSensitiveDetector::GSensitiveDetector(string sdName,
                                       GOptions *goptions,
                                       map<string, GDynamicDigitization *> *gDDGlobal) :
        G4VSensitiveDetector(sdName),                                                                        // geant4 derived
        GStateMessage(goptions, "GSensitiveDetector " + sdName, "gsensitivityv"),  // GStateMessage derived
        gDynamicDigitizationMapGlobalInstance(gDDGlobal),
        gHitsCollection(nullptr) {

    verbosity = goptions->getInt("gsensitivityv");
    recordZeroEdep = goptions->getSwitch("recordZeroEdep");

    // protected, from G4VSensitiveDetector: it's a G4CollectionNameVector
    // not really used in gemc but it's no overhead to do it as in the examples
    string hitCollectionName = sdName + "__HitCollection";
    collectionName.insert(hitCollectionName);

    logSummary("Instantiating GSensitiveDetector " + sdName);
}


// thread local
// run at the beginning of the event
void GSensitiveDetector::Initialize(G4HCofThisEvent *g4hc) {

    string sdName = GetName();
    logSummary("Initialize GSensitiveDetector " + sdName);

    // assigning thread local DynamicDigitization from the global map
    if (gDynamicDigitizationMapGlobalInstance->find(sdName) != gDynamicDigitizationMapGlobalInstance->end()) {
        gDynamicDigitizationLocalInstance = (*gDynamicDigitizationMapGlobalInstance)[sdName];
        gHitBitSet = gDynamicDigitizationLocalInstance->readoutSpecs->getHitBitSet();
    }

    // protecting against plugin loading failures
    if (!gDynamicDigitizationLocalInstance) {
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
G4bool GSensitiveDetector::ProcessHits(G4Step *thisStep, [[maybe_unused]] G4TouchableHistory *g4th) {

    // gDynamicDigitizationLocalInstance is guaranteed to exist in GSensitiveDetector::Initialize

    // if there is a decision to skip this event based on depe, return
    double depe = thisStep->GetTotalEnergyDeposit();
    if (decisionToSkipHit(depe)) {
        return true;
    }

    // get the vector of GTouchables returned by gDynamicDigitizationLocalInstance
    // if not defined by the plugin, base class will return a vector with one element (the input)
    vector<GTouchable *> thisStepProcessedTouchables = gDynamicDigitizationLocalInstance->processTouchable(getGTouchable(thisStep), thisStep);

    if (verbosity >= GVERBOSITY_SUMMARY) {
        G4cout << " Starting processHits step loop with " << thisStepProcessedTouchables.size()
               << " touchable(s) for step pointer" << thisStep;
        G4cout << ", edep: " << depe << ", Hit collection size: " << gHitsCollection->GetSize() << G4endl;
    }

    for (const auto thisGTouchable: thisStepProcessedTouchables) {

        // assign track id
        thisGTouchable->assignTrackId(thisStep->GetTrack()->GetTrackID());

        if (isThisANewTouchable(thisGTouchable)) {
            // new ghit, insert in gHitsCollection
            // the constructor takes care of the filling the hit step infos according to gHitBitSet
            gHitsCollection->insert(new GHit(thisGTouchable, thisStep, gHitBitSet));
        } else {
            // not a new touchable, must be in hit collection
            // retrieve hit from hit collection
            // add information according to gHitBitSet
            GHit *existingHit = getHitInHitCollectionUsingTouchable(thisGTouchable);
            existingHit->addHitInfosForBitset(gHitBitSet, thisStep);
        }
    }


    return true;
}

// checking if it is present in the set. If not, add it.
bool GSensitiveDetector::isThisANewTouchable(const GTouchable *thisTouchable) {

    if (verbosity >= GVERBOSITY_SUMMARY) {
        G4cout << " isThisANewTouchable for " << *thisTouchable;
    }

    // if not found insert and return true: it's a new
    auto gtPosition = find(touchableVector.begin(), touchableVector.end(), *thisTouchable);

    if (gtPosition == touchableVector.end()) {
        if (verbosity >= GVERBOSITY_DETAILS) {
            G4cout << " ✅ yes, new GTouchable. Adding it to touchableVector. " << G4endl;
        }
        touchableVector.push_back(*thisTouchable);
        return true;
    } else {
        if (verbosity >= GVERBOSITY_DETAILS) {
            G4cout << " ❌ no, GTouchable is found, retrieving hit...";
        }
        return false;
    }
}


GHit *GSensitiveDetector::getHitInHitCollectionUsingTouchable(const GTouchable *gtouchable) {

    for (unsigned int i = 0; i < gHitsCollection->GetSize(); i++) {

        GHit *thisHit = (*gHitsCollection)[i];
        const GTouchable *thisHitGTouchable = thisHit->getGTouchable();

        if (*gtouchable == *thisHitGTouchable) {
            if (verbosity >= GVERBOSITY_DETAILS) {
                G4cout << " getHitInHitCollectionUsingTouchable ✅ - adding hit infos for existingHit touchable" << G4endl;
            }
            return thisHit;
        }
    }

    G4cout << G4endl << " getHitInHitCollectionUsingTouchable GTouchable not found error:"
           << " it should be there because isThisANewTouchable was true.  getHitInHitCollectionUsingTouchable loop:"
           << G4endl;

    for (unsigned int i = 0; i < gHitsCollection->GetSize(); i++) {

        GHit *thisHit = (*gHitsCollection)[i];
        const GTouchable *thisHitGTouchable = thisHit->getGTouchable();

        G4cout << " getHitInHitCollectionUsingTouchable Hit n. " << i
               << "  comparing  candidate " << G4endl
               << *gtouchable
               << " with touchable in collection " << G4endl
               << *thisHitGTouchable
               << " comparison result : " << (*gtouchable == *thisHitGTouchable) << G4endl;
    }


    // hit not found, error: exit
    logError("GHit not found in hit collection", EC__HITNOTFOUNDINCOLLECTION);

    return nullptr;
}


// thread local
void GSensitiveDetector::EndOfEvent([[maybe_unused]] G4HCofThisEvent *g4hc) {
    logSummary("EndOfEvent of GSensitiveDetector " + GetName());
}


// called in GDetectorConstruction::ConstructSDandField
void GSensitiveDetector::registerGVolumeTouchable(string name, GTouchable *gt) {
    // not using log here as we're couting gt
    if (verbosity == GVERBOSITY_DETAILS) {
        G4cout << "Registering touchable gvolume <" << name << "> with  value: " << gt << G4endl;
    }
    gTouchableMap[name] = gt;
}
