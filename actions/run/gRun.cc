// geant4
#include "G4Event.hh"

// gemc
#include "gRun.h"
#include "ghit.h"
//#include "event/gEventHeader.h"


// Constructor
GRun::GRun(std::shared_ptr<GOptions> gopt, std::shared_ptr<gdynamicdigitization::dRoutinesMap> digi_map) :
	digitization_routines_map(digi_map) {
	auto desc = "GRun " + std::to_string(G4Threading::G4GetThreadId());
	log       = std::make_shared<GLogger>(gopt, GRUN_LOGGER, desc);

	log->debug(CONSTRUCTOR, FUNCTION_NAME);
}

// Destructor
GRun::~GRun() { log->debug(DESTRUCTOR, FUNCTION_NAME); }

using GHitsCollection = G4THitsCollection<GHit>;

// RecordEvent is called at the end of every event
// Method to be overwritten by the user for recording events in this (thread-local) run.
// The observables defined in each run should be filled here with the information from the hits
void GRun::RecordEvent(const G4Event* aEvent) {
	int thread_id = G4Threading::G4GetThreadId();
	int eventID   = aEvent->GetEventID();

	log->debug(CONSTRUCTOR, FUNCTION_NAME);

	log->info(2, FUNCTION_NAME, " worker n. ", thread_id, " for event number ", eventID);


	// HitsCollections of This Event
	G4HCofThisEvent* HCsThisEvent = aEvent->GetHCofThisEvent();
	if (!HCsThisEvent) return;


	// PRAGMA TODO: see https://twiki.cern.ch/twiki/bin/view/Geant4/Geant4MTTipsAndTricks#11_How_are_exactly_random_number
	// currently this returns null, but perhaps need some activating?
	// Or we need to save the random status in separate file ?
	//	string randomStatus = UNINITIALIZEDSTRINGQUANTITY;
	//	if(aEvent->GetRandomNumberStatusForProcessing()) {
	//		randomStatus = aEvent->GetRandomNumberStatusForProcessing();
	//	}

	// header
	auto gheader   = std::make_unique<GEventHeader>(eventID, thread_id, log); // g4run-local event number
	auto eventData = std::make_shared<GEventDataCollection>(std::move(gheader), log);

	// looping over all collections
	//	for (G4int hci = 0; hci < HCsThisEvent->GetNumberOfCollections(); hci++) {
	//	GHitsCollection* thisGHC = (GHitsCollection*)HCsThisEvent->GetHC(hci);

	//	if (thisGHC) {
	// std::string hitCollectionSDName = thisGHC->GetSDname();
	//
	// log->info(2, FUNCTION_NAME, " worker n. ", thread_id,
	//           " for event number ", eventID,
	//           " for collection number ", hci + 1,
	//           " collection name: ", hitCollectionSDName);
	//
	// auto digitization_routine = digitization_routines_map->at(hitCollectionSDName);


	// if (digitization_routine != nullptr) {
	// 	// looping over hits in this collection
	// 	for (size_t hitIndex = 0; hitIndex < thisGHC->GetSize(); hitIndex++) {
	//
	// 		auto thisHit = (GHit*)thisGHC->GetHit(hitIndex);
	//
	// 		// PRAGMA TODO: switch these on/off with options
	// 		auto true_data = digitization_routine->collectTrueInformation(thisHit, hitIndex);
	// 		auto digi_data = digitization_routine->digitizeHit(thisHit, hitIndex);
	//
	// 		eventData->addDetectorDigitizedData(hitCollectionSDName, std::move(digi_data));
	// 		eventData->addDetectorTrueInfoData(hitCollectionSDName, std::move(true_data));
	//
	// 	}
	// }


	//	}
	//	}

	//runData.push_back(eventDataCollection);
}

// This is global
// Method to be overwritten by the user for merging local Run objects to the global Run object
// I can use it to save output right? No! Need to accumulate. Writing output should go in GRunAction::EndOfRunAction!
// void GRun::Merge(const G4Run* aRun) {
// 	const GRun* localRun = static_cast<const GRun*>(aRun);
//
// 	for (auto run : localRun->runData) { runData.push_back(run); }
//
// 	if (verbosity >= GVERBOSITY_DETAILS) {
// 		G4cout << GEMCRUNHEADER << "GRun:Global Merge" << G4endl;
// 		G4cout << GEMCRUNHEADER << "GRun: local run data size " << localRun->runData.size() << "  global size: " << runData.size() << G4endl;
// 	}
//
// 	G4Run::Merge(aRun);
// }
