// gemc
#include "gRun.h"
#include "gemcConventions.h"

// geant4
#include "G4Event.hh"

// glibrary
#include "ghit.h"
#include "event/gEventDataCollectionHeader.h"
#include "gutsConventions.h"


// Constructor
GRun::GRun(GOptions *gopt, map<string, GDynamicDigitization *> *gDDGlobal) :
        G4Run(),
        gDigitizationGlobalMap(gDDGlobal) {
    verbosity = gopt->getVerbosityFor("grun");
    elog = gopt->getScalarInt("em_log");

    if (verbosity >= GVERBOSITY_CLASSES) {
        gLogClassConstruct("GRun Constructor ");
    }
}

// Destructor
GRun::~GRun() {
    if (verbosity >= GVERBOSITY_CLASSES) {
        gLogClassDestruct("GRun Destructor");
    }

    // data is deleted in GRunAction::EndOfRunAction
}

using GHitsCollection = G4THitsCollection<GHit>;

// RecordEvent is called at end of every event
// Method to be overwritten by the user for recording events in this (thread-local) run.
// The observables defined in each run should be filled here with the information from the hits
void GRun::RecordEvent(const G4Event *aEvent) {

    if (verbosity >= GVERBOSITY_DETAILS) {

        int eventID = aEvent->GetEventID();

        if (eventID % elog == 0) {
            G4cout << GEMCRUNHEADER << "GRun:Local RecordEvent for number " << eventID << G4endl;
        }
    }
    // HitsCollections of This Event
    G4HCofThisEvent *HCsThisEvent = aEvent->GetHCofThisEvent();
    if (!HCsThisEvent) return;


    // PRAGMA TODO: see https://twiki.cern.ch/twiki/bin/view/Geant4/Geant4MTTipsAndTricks#11_How_are_exactly_random_number
    // currently this returns null, but perhaps need some activating?
    // Or we need to save the random status in separate file ?
    //	string randomStatus = UNINITIALIZEDSTRINGQUANTITY;
    //	if(aEvent->GetRandomNumberStatusForProcessing()) {
    //		randomStatus = aEvent->GetRandomNumberStatusForProcessing();
    //	}

    // header
    GEventDataCollectionHeader *gheader = new GEventDataCollectionHeader(aEvent->GetEventID(),           // g4run-local event number
                                                                         G4Threading::G4GetThreadId(),   // thread ID
                                                                         verbosity);

    // thread-local event data
    // collects hits in the entire event
    GEventDataCollection *eventDataCollection = new GEventDataCollection(gheader, verbosity);

    // looping over all collections
    for (G4int hci = 0; hci < HCsThisEvent->GetNumberOfCollections(); hci++) {

        GHitsCollection *thisGHC = (GHitsCollection *) HCsThisEvent->GetHC(hci);

        if (thisGHC) {

            if (verbosity >= GVERBOSITY_DETAILS) {
                G4cout << " Collection number  " << hci + 1 << " has pointer <" << thisGHC << "> and name <" << thisGHC->GetName() << ">" << G4endl;
            }

            string hitCollectionSDName = thisGHC->GetSDname();

            // getting digitization for this hit collection
            GDynamicDigitization *detectorDigitization = getDigitizationForHitCollection(hitCollectionSDName);

            if (detectorDigitization != nullptr) {

                // looping over hits in this collection
                for (size_t hitIndex = 0; hitIndex < thisGHC->GetSize(); hitIndex++) {
                    GHit *thisHit = (GHit *) thisGHC->GetHit(hitIndex);

                    // digitize hit and add it to detector data
                    // PRAGMA TODO: switch this on/off with option

                    eventDataCollection->addDetectorTrueInfoData(hitCollectionSDName, detectorDigitization->collectTrueInformation(thisHit, hitIndex + 1));
                    eventDataCollection->addDetectorDigitizedData(hitCollectionSDName, detectorDigitization->digitizeHit(thisHit, hitIndex + 1));
                }
            }
        }
    }

    runData.push_back(eventDataCollection);
    G4Run::RecordEvent(aEvent);
    // not deleting the pointers: they are being merged below
}

// This is global
// Method to be overwritten by the user for merging local Run objects to the global Run object
// I can use it to save output right? No! Need to accumulate. Writing output should go in GRunAction::EndOfRunAction!
void GRun::Merge(const G4Run *aRun) {
    const GRun *localRun = static_cast<const GRun *> (aRun);

    for (auto run: localRun->runData) {
        runData.push_back(run);
    }

    if (verbosity >= GVERBOSITY_DETAILS) {
        G4cout << GEMCRUNHEADER << "GRun:Global Merge" << G4endl;
        G4cout << GEMCRUNHEADER << "GRun: local run data size " << localRun->runData.size() << "  global size: " << runData.size() << G4endl;
    }

    G4Run::Merge(aRun);
}


// retrieve digitization from gDigitizationGlobal map
GDynamicDigitization *GRun::getDigitizationForHitCollection(string name) {
    if (gDigitizationGlobalMap->find(name) == gDigitizationGlobalMap->end()) {
        return nullptr;
    }

    return (*gDigitizationGlobalMap)[name];
}
