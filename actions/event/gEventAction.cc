#include "gEventAction.h"
#include "actions/gactionConventions.h"

// geant4
#include "G4EventManager.hh"

// gemc
#include "event/gEventDataCollection.h"


GEventAction::GEventAction(const std::shared_ptr<GOptions>& gopt, GRunAction* run_a) :
	GBase(gopt, EVENTACTION_LOGGER),
	goptions(gopt),
	run_action(run_a) {
	// Constructor: store shared config and a non-owning pointer to the run action for this thread.
	auto desc = "GEventAction " + std::to_string(G4Threading::G4GetThreadId());
	log->debug(CONSTRUCTOR, FUNCTION_NAME, desc);
}

void GEventAction::BeginOfEventAction([[maybe_unused]] const G4Event* event) {
	// Begin-of-event hook: logs event id and thread id for tracing.
	int thread_id = G4Threading::G4GetThreadId();
	int eventID   = event->GetEventID();

	log->debug(CONSTRUCTOR, FUNCTION_NAME, " event id ", eventID, " in thread ", thread_id);
}

void GEventAction::EndOfEventAction([[maybe_unused]] const G4Event* event) {
	// End-of-event hook: collect hit collections, digitize hits, and publish the event to streamers.
	G4HCofThisEvent* HCsThisEvent = event->GetHCofThisEvent();
	if (!HCsThisEvent) return;
	if (!run_action) {
		log->error(ERR_GRUNACTION_NOT_EXISTING, FUNCTION_NAME,
		           " run_action is null - cannot access digitization routines or streamers.");
	}

	int thread_id = G4Threading::G4GetThreadId();
	int eventID   = event->GetEventID();

	// Create the event data container that will receive digitized data and truth information.
	auto gevent_header       = std::make_unique<GEventHeader>(goptions, eventID, thread_id);
	auto eventDataCollection = std::make_shared<GEventDataCollection>(goptions, std::move(gevent_header));

	// Loop over all hit collections produced by sensitive detectors in this event.
	for (G4int hci = 0; hci < HCsThisEvent->GetNumberOfCollections(); hci++) {
		auto thisGHC = (GHitsCollection*)HCsThisEvent->GetHC(hci);

		if (thisGHC) {
			std::string hcSDName = thisGHC->GetSDname();
			auto        digi_map = run_action->get_digitization_routines_map();
			if (!digi_map) {
				log->error(ERR_GDIGIMAP_NOT_EXISTING, FUNCTION_NAME,
				           " no digitization routines map available for collection ", hcSDName,
				           " in thread ", thread_id);
			}

			log->info(2, FUNCTION_NAME, " worker ", thread_id,
			          " for event number ", eventID,
			          " for collection number ", hci + 1,
			          " collection name: ", hcSDName);

			// Select the digitization routine by hit collection name, then publish through all streamers.
			auto it = digi_map->find(hcSDName);
			if (it == digi_map->end()) {
				log->error(ERR_GDIGIMAP_NOT_EXISTING, FUNCTION_NAME,
				           " no digitization routine registered for collection ", hcSDName,
				           " in thread ", thread_id);
			}
			auto digitization_routine = it->second;

			auto gstreamers_map = run_action->get_streamer_map();
			if (!gstreamers_map) {
				log->error(ERR_STREAMERMAP_NOT_EXISTING, FUNCTION_NAME, " no gstreamer map available in thread ",
				           thread_id);
			}

			if (digitization_routine != nullptr) {
				// Loop over hits in this collection and append produced data to the event container.
				for (size_t hitIndex = 0; hitIndex < thisGHC->GetSize(); hitIndex++) {
					auto thisHit = (GHit*)thisGHC->GetHit(hitIndex);
					// PRAGMA TODO: switch these on/off with options
					auto true_data = digitization_routine->collectTrueInformation(thisHit, hitIndex);
					auto digi_data = digitization_routine->digitizeHit(thisHit, hitIndex);
					eventDataCollection->addDetectorDigitizedData(hcSDName, std::move(digi_data));
					eventDataCollection->addDetectorTrueInfoData(hcSDName, std::move(true_data));
				}

				for (const auto& [name, gstreamer] : *gstreamers_map) {
					// Publish the event to the gstreamer.
					gstreamer->publishEventData(eventDataCollection);
				}
			}
		}
	}
}
