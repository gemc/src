#include "gEventAction.h"
#include "actions/gactionConventions.h"

// geant4
#include "G4EventManager.hh"
#include "G4Threading.hh"

// gemc
#include "event/gEventDataCollection.h"


// Construct the event action and keep access to shared configuration plus the
// non-owning thread-local run action used during event finalization.
GEventAction::GEventAction(const std::shared_ptr<GOptions>& gopt, GRunAction* run_a) :
	GBase(gopt, EVENTACTION_LOGGER),
	goptions(gopt),
	run_action(run_a) {
	const auto desc = "GEventAction " + std::to_string(G4Threading::G4GetThreadId());
	log->debug(CONSTRUCTOR, FUNCTION_NAME, desc);
}

// Begin-of-event hook used mainly for tracing event and thread identifiers.
void GEventAction::BeginOfEventAction([[maybe_unused]] const G4Event* event) {
	const auto thread_id = G4Threading::G4GetThreadId();
	const auto event_id  = event->GetEventID();

	log->debug(NORMAL, FUNCTION_NAME, " event id ", event_id, " in thread ", thread_id);
}

// Finalize the event by reading hit collections, digitizing them, routing the
// resulting payload according to collection mode, and publishing event-mode output.
void GEventAction::EndOfEventAction([[maybe_unused]] const G4Event* event) {
	if (run_action == nullptr) {
		log->error(ERR_GRUNACTION_NOT_EXISTING, FUNCTION_NAME,
				   " run_action is null - cannot access digitization routines or streamers.");
		return;
	}

	// Count each processed event once, even when it produces no payload.
	run_action->increment_run_events_processed();

	auto* const hcs_this_event = event->GetHCofThisEvent();
	if (hcs_this_event == nullptr) {
		return;
	}

	const auto thread_id = G4Threading::G4GetThreadId();
	const auto event_id  = event->GetEventID();

	auto gevent_header       = std::make_unique<GEventHeader>(goptions, event_id, thread_id);
	auto eventDataCollection = std::make_shared<GEventDataCollection>(goptions, std::move(gevent_header));

	const auto digi_map = run_action->get_digitization_routines_map();
	if (digi_map == nullptr) {
		log->error(ERR_GDIGIMAP_NOT_EXISTING, FUNCTION_NAME,
				   " no digitization routines map available in thread ", thread_id);
		return;
	}

	bool has_event_mode_payload = false;
	bool has_run_mode_payload   = false;

	// Loop over every hit collection produced during this event and dispatch each
	// collection to the digitization routine registered under its collection name.
	for (G4int hci = 0; hci < hcs_this_event->GetNumberOfCollections(); ++hci) {
		auto* const this_ghc = static_cast<GHitsCollection*>(hcs_this_event->GetHC(hci));
		if (this_ghc == nullptr) {
			continue;
		}

		const std::string hcSDName = this_ghc->GetSDname();

		log->info(2, FUNCTION_NAME, " worker ", thread_id,
				  " for event number ", event_id,
				  " for collection number ", hci + 1,
				  " collection name: ", hcSDName);

		// Resolve the digitization routine responsible for this collection.
		const auto it = digi_map->find(hcSDName);
		if (it == digi_map->end()) {
			log->error(ERR_GDIGIMAP_NOT_EXISTING, FUNCTION_NAME,
					   " no digitization routine registered for collection ", hcSDName,
					   " in thread ", thread_id);
			continue;
		}

		const auto& digitization_routine = it->second;
		if (digitization_routine == nullptr) {
			log->error(ERR_GDIGIMAP_NOT_EXISTING, FUNCTION_NAME,
					   " digitization routine is null for collection ", hcSDName,
					   " in thread ", thread_id);
			continue;
		}

		const auto collection_mode = digitization_routine->collection_mode();

		// Process all hits in the collection. Event-mode digitizers append to the
		// event container, while run-mode digitizers append to the run container.
		for (size_t hitIndex = 0; hitIndex < this_ghc->GetSize(); ++hitIndex) {
			auto* const this_hit = static_cast<GHit*>(this_ghc->GetHit(hitIndex));
			if (this_hit == nullptr) {
				continue;
			}

			auto true_data = digitization_routine->collectTrueInformation(this_hit, hitIndex);
			auto digi_data = digitization_routine->digitizeHit(this_hit, hitIndex);

			if (collection_mode == CollectionMode::event) {
				eventDataCollection->addDetectorDigitizedData(hcSDName, std::move(digi_data));
				eventDataCollection->addDetectorTrueInfoData(hcSDName, std::move(true_data));
				has_event_mode_payload = true;
			}
			else if (collection_mode == CollectionMode::run) {
				if (digi_data != nullptr) {
					run_action->collect_event_data_collections(
						hcSDName,
						std::move(digi_data));
					has_run_mode_payload = true;
				}
			}
		}
	}

	// Record whether this event contributed at least one run-mode payload entry.
	if (has_run_mode_payload) {
		run_action->increment_run_events_with_payload();
	}

	// Publish event-mode output once, after all collections have been processed.
	if (has_event_mode_payload) {
		publish_event_data(eventDataCollection);
	}
}

// Send the completed event-data object to every configured worker-thread streamer.
void GEventAction::publish_event_data(const std::shared_ptr<GEventDataCollection>& event_data) const {
	if (run_action == nullptr || event_data == nullptr) {
		return;
	}

	const auto gstreamers_threads_map = run_action->get_streamer_threads_map();
	if (gstreamers_threads_map == nullptr) {
		log->error(ERR_STREAMERMAP_NOT_EXISTING, FUNCTION_NAME,
				   " no thread streamer map available - event will not be published.");
		return;
	}

	for (const auto& [name, gstreamer] : *gstreamers_threads_map) {
		if (gstreamer == nullptr) {
			log->error(ERR_STREAMERMAP_NOT_EXISTING, FUNCTION_NAME,
					   " null gstreamer instance for streamer ", name);
			continue;
		}

		gstreamer->publishEventData(event_data);
	}
}