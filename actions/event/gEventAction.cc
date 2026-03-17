#include "gEventAction.h"
#include "actions/gactionConventions.h"

// geant4
#include "G4EventManager.hh"
#include "G4Threading.hh"

// gemc
#include "event/gEventDataCollection.h"


GEventAction::GEventAction(const std::shared_ptr<GOptions>& gopt, GRunAction* run_a) :
	GBase(gopt, EVENTACTION_LOGGER),
	goptions(gopt),
	run_action(run_a) {
	// Constructor: store shared config and a non-owning pointer to the run action for this thread.
	const auto desc = "GEventAction " + std::to_string(G4Threading::G4GetThreadId());
	log->debug(CONSTRUCTOR, FUNCTION_NAME, desc);
}

void GEventAction::BeginOfEventAction([[maybe_unused]] const G4Event* event) {
	// Begin-of-event hook: log event id and thread id for tracing.
	const auto thread_id = G4Threading::G4GetThreadId();
	const auto event_id  = event->GetEventID();

	log->debug(NORMAL, FUNCTION_NAME, " event id ", event_id, " in thread ", thread_id);
}

void GEventAction::EndOfEventAction([[maybe_unused]] const G4Event* event) {
	// End-of-event hook: collect hit collections, digitize hits, and publish the event to streamers.
	auto* const hcs_this_event = event->GetHCofThisEvent();
	if (hcs_this_event == nullptr) {
		return;
	}

	if (run_action == nullptr) {
		log->error(ERR_GRUNACTION_NOT_EXISTING, FUNCTION_NAME,
				   " run_action is null - cannot access digitization routines or streamers.");
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

	// Loop over all hit collections produced by sensitive detectors in this event.
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

		// Select the digitization routine by hit collection name.
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

		// Loop over hits in this collection and append produced data to the appropriate container.
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
				run_action->collect_event_data_collections(
					hcSDName,
					std::move(digi_data));
			}
		}
	}

	// Publish once per event, after all event-mode collections have been processed.
	if (has_event_mode_payload) {
		publish_event_data(eventDataCollection);
	}
}

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
