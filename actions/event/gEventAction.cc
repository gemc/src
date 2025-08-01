#include "gEventAction.h"

// geant4
#include "G4EventManager.hh"

// gemc
#include "event/gEventHeader.h"


GEventAction::GEventAction(std::shared_ptr<GOptions> gopt, GRunAction* run_a) : run_action(run_a) {
	auto desc = "GEventAction " + std::to_string(G4Threading::G4GetThreadId());
	log       = std::make_shared<GLogger>(gopt, EVENTACTION_LOGGER, desc);

	log->debug(CONSTRUCTOR, FUNCTION_NAME);
}

GEventAction::~GEventAction() { log->debug(DESTRUCTOR, FUNCTION_NAME); }

void GEventAction::BeginOfEventAction([[maybe_unused]] const G4Event* event) {
	int thread_id = G4Threading::G4GetThreadId();
	int eventID   = event->GetEventID();

	log->debug(CONSTRUCTOR, FUNCTION_NAME, " event id ", eventID, " in thread ", thread_id);


}

void GEventAction::EndOfEventAction([[maybe_unused]] const G4Event* event) {
	G4HCofThisEvent* HCsThisEvent = event->GetHCofThisEvent();
	if (!HCsThisEvent) return;

	int thread_id = G4Threading::G4GetThreadId();
	int eventID   = event->GetEventID();

	auto gheader   = std::make_unique<GEventHeader>(eventID, thread_id, log);
	auto eventData = std::make_shared<GEventDataCollection>(std::move(gheader), log);

	// looping over all collections
	for (G4int hci = 0; hci < HCsThisEvent->GetNumberOfCollections(); hci++) {
		GHitsCollection* thisGHC = (GHitsCollection*)HCsThisEvent->GetHC(hci);

		if (thisGHC) {
			std::string hitCollectionSDName = thisGHC->GetSDname();
			auto digi_map = run_action->get_digitization_routines_map();

			log->info(2, FUNCTION_NAME, " worker ", thread_id,
			          " for event number ", eventID,
			          " for collection number ", hci + 1,
			          " collection name: ", hitCollectionSDName);

			auto digitization_routine = digi_map->at(hitCollectionSDName);
			auto gstreamers_map = run_action->get_streamer_map();


			if (digitization_routine != nullptr) {

				// looping over hits in this collection
				for (size_t hitIndex = 0; hitIndex < thisGHC->GetSize(); hitIndex++) {
					auto thisHit = (GHit*)thisGHC->GetHit(hitIndex);
					// PRAGMA TODO: switch these on/off with options
					auto true_data = digitization_routine->collectTrueInformation(thisHit, hitIndex);
					auto digi_data = digitization_routine->digitizeHit(thisHit, hitIndex);
					eventData->addDetectorDigitizedData("ctof", std::move(digi_data));
					eventData->addDetectorTrueInfoData("ctof", std::move(true_data));
				}

				for (const auto& [name, gstreamer] : *gstreamers_map) {
					// publish the event to the gstreamer
					gstreamer->publishEventData(eventData);
				}
			}
		}
	}
}
