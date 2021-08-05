// gemc
#include "gRun.h"

// geant4
#include "G4Event.hh"

// glibrary
#include "ghit.h"
#include "event/gEventDataCollectionHeader.h"


// Constructor
GRun::GRun(GOptions* gopt, map<string, GDynamicDigitization*> *gDDGlobal, map<string, GStreamer*> *gstrFactory) :
G4Run(),
GStateMessage(gopt, "GRun ", "grunv"),  // GStateMessage derived
gDigitizationGlobalMap(gDDGlobal),
gstreamerFactoryMap(gstrFactory)
{
	logSummary("Instantiating GRun ");

	runData = new vector<GEventDataCollection*>;
}

// Destructor
GRun::~GRun()
{
	logSummary("GRun:Destructor");
	
	// PRAGMA TODO: isn't the last line enough?
	for (GEventDataCollection* evtData : *runData) {
		delete evtData;
	}
	
	runData->clear(); // i think this is not enough. we need to go over each member of ALL the maps and delete

	//  pointer
	delete runData;
}

using GHitsCollection = G4THitsCollection<GHit> ;

// RecordEvent is called at end of every event
// Method to be overwritten by the user for recording events in this (thread-local) run.
// The observables defined in each run should be filled here with the information from the hits
void GRun::RecordEvent(const G4Event *aEvent)
{
	logSummary("GRun:Local RecordEvent");
	// HitsCollections of This Event
	G4HCofThisEvent* HCsThisEvent = aEvent->GetHCofThisEvent();
	if(!HCsThisEvent) return;
	
	string randomStatus = "na";
	// PRAGMA TODO: see https://twiki.cern.ch/twiki/bin/view/Geant4/Geant4MTTipsAndTricks#11_How_are_exactly_random_number
	// currently this returns null, but perhaps need some activating?
	// Or we need to save the random status in separate file ?
//	if(aEvent->GetRandomNumberStatusForProcessing()) {
//		randomStatus = aEvent->GetRandomNumberStatusForProcessing();
//	}
	
	// header
	int verbosity = 0;
	GEventDataCollectionHeader *gheader = new GEventDataCollectionHeader(aEvent->GetEventID(),                   // local event number
														  G4Threading::G4GetThreadId(),           // thread ID
														  verbosity);

	// thread-local event data
	GEventDataCollection *eventDataCollection = new GEventDataCollection(gheader, verbosity);
	
	// looping over all collections
	for(unsigned hci = 0; hci < HCsThisEvent->GetNumberOfCollections(); hci++) {
		
		GHitsCollection *thisGHC = (GHitsCollection*) HCsThisEvent->GetHC(hci);
		
		if(thisGHC) {
			
			// G4cout << " Collection number  " << hci + 1 << " " << thisGHC << " name " << thisGHC->GetSDname() <<  G4endl ;
			
			string hitCollectionSDName = thisGHC->GetSDname();
			
			// getting digitization for this hit collection
			GDynamicDigitization* detectorDigitization = getDigitizationForHitCollection(hitCollectionSDName);
			
			if(detectorDigitization != nullptr) {

				// looping over hits in this collection
				for(size_t hitIndex = 0; hitIndex<thisGHC->GetSize(); hitIndex++) {
					GHit *thisHit = (GHit*) thisGHC->GetHit(hitIndex);
//
//					// digitize hit and add it to detector data
//					// PRAGMA TODO: switch this on/off with option
//					detectorObservables->addDetectorObservables(detectorDigitization->digitizeHit(thisHit));
//
//					// digitize true info and add it to detector data
//					// PRAGMA TODO: switch this on/off with option
//					detectorObservables->addDetectorObservables(detectorDigitization->trueInfoHit(thisHit), true);
//
				}
//				eventData->addDetectorData(detectorObservables);


			}
		}
	}
	
	runData->push_back(eventDataCollection);
	G4Run::RecordEvent(aEvent);
}

// This is global
// Method to be overwritten by the user for merging local Run objects to the global Run object
// PRAGMA: But I can use it to save output right? No need to accumulate
void GRun::Merge(const G4Run *aRun)
{
	logSummary("GRun:Global Merge");

	const GRun *localRun = static_cast<const GRun *> (aRun);

//	//	cout << " local run data size " << localRun->runData->size() << "  global size: " << runData->size() << endl;
//
//	// output data to all available plugins
//	for(auto gmf: (*gmediaFactory)) {
//		// protecting against DL failure
//		if(gmf.second != nullptr) {
//			gmf.second->publishData(localRun->runData);
//		}
//	}

	G4Run::Merge(aRun);
}



// retrieve digitization from gDigitizationGlobal map
GDynamicDigitization* GRun::getDigitizationForHitCollection(string name)
{
	if(gDigitizationGlobalMap->find(name) == gDigitizationGlobalMap->end()) {
		return nullptr;
	}
	
	return (*gDigitizationGlobalMap)[name];
}

