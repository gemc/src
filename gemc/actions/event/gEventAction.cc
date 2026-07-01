#include "gEventAction.h"
#include "../gactionConventions.h"

// geant4
#include "G4Event.hh"
#include "G4Threading.hh"

// c++
#include <string>

// gemc
#include "event/gEventDataCollection.h"
#include "../generator/gPrimaryGeneratorAction.h"
#include "../tracking/gTrackProvenance.h"

// c++
#include <algorithm>
#include <cctype>
#include <chrono>
#include <unordered_set>

namespace {
GGeneratedParticleBank make_generated_particle_bank(const GParticleRecordEvent& particles) {
	GGeneratedParticleBank bank;
	bank.reserve(particles.size());

	for (const auto& particle : particles) {
		bank.push_back({
			particle.name,
			particle.pid,
			particle.type,
			particle.multiplicity,
			particle.p,
			particle.theta,
			particle.phi,
			particle.vx,
			particle.vy,
			particle.vz
		});
	}

	return bank;
}

GAncestorBank make_ancestor_bank(const std::vector<GTrackRecord>& records) {
	GAncestorBank bank;
	bank.reserve(records.size());
	for (const auto& record : records) {
		bank.push_back({
			record.pid,
			record.tid,
			record.mtid,
			record.kinetic_energy,
			record.momentum.x(),
			record.momentum.y(),
			record.momentum.z(),
			record.vertex.x(),
			record.vertex.y(),
			record.vertex.z()
		});
	}
	return bank;
}

bool scalar_bool_option_enabled(const std::shared_ptr<GOptions>& goptions, const std::string& name) {
	std::string value = goptions->getScalarString(name);
	std::transform(value.begin(), value.end(), value.begin(),
	               [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
	return value == "true" || value == "1" || value == "yes" || value == "on";
}

// Convert a substring to a non-negative integer, returning false on any malformed input.
bool to_non_negative_int(const std::string& text, int& out) {
	if (text.empty()) return false;
	size_t pos = 0;
	int    value;
	try { value = std::stoi(text, &pos); }
	catch (...) { return false; }
	if (pos != text.size() || value < 0) return false;
	out = value;
	return true;
}
}


// Construct the event action and keep access to shared configuration plus the
// non-owning thread-local run action used during event finalization.
GEventAction::GEventAction(const std::shared_ptr<GOptions>& gopt, GRunAction* run_a,
	                       std::shared_ptr<GTrackProvenance> provenance) :
	GBase(gopt, EVENTACTION_LOGGER),
	goptions(gopt),
	run_action(run_a),
	track_provenance(std::move(provenance)) {
	const auto thread_id = G4Threading::G4GetThreadId();
	const auto desc      = "GEventAction " + std::to_string(thread_id);
	log->debug(CONSTRUCTOR, FUNCTION_NAME, desc);
	save_all_ancestors  = goptions->getSwitch(SAVE_ALL_ANCESTORS_SWITCH);
	save_original_track = goptions->getSwitch(SAVE_ORIGINAL_TRACK_SWITCH) || save_all_ancestors;

	// Parse the log_every option of the form N or N-NTH. Anything malformed disables the
	// feature and is reported once (from thread 0) to avoid duplicated warnings across workers.
	std::string spec = goptions->getScalarString(LOG_EVERY_OPTION);
	if (!spec.empty() && spec != UNINITIALIZEDSTRINGQUANTITY) {
		const auto dash = spec.find('-');
		std::string n_part   = dash == std::string::npos ? spec : spec.substr(0, dash);
		std::string nth_part = dash == std::string::npos ? std::string() : spec.substr(dash + 1);

		// Effective worker-thread count, mirroring gemc::get_nthreads clamping (0 means all cores).
		int nthreads = goptions->getScalarInt("nthreads");
		const int ncores = G4Threading::G4GetNumberOfCores();
		if (nthreads == 0 || nthreads > ncores) nthreads = ncores;

		int n = 0;
		if (!to_non_negative_int(n_part, n) || n == 0) {
			if (thread_id <= 0)
				log->warning("Ignoring invalid -", LOG_EVERY_OPTION, "=", spec,
				             " : N must be a positive integer.");
		}
		else if (dash != std::string::npos) {
			int nth = 0;
			if (!to_non_negative_int(nth_part, nth) || nth >= nthreads) {
				if (thread_id <= 0)
					log->warning("Ignoring invalid -", LOG_EVERY_OPTION, "=", spec,
					             " : thread id must be in [0, ", nthreads - 1, "].");
			}
			else {
				log_every_n      = n;
				log_every_thread = nth;
			}
		}
		else { log_every_n = n; }
	}
}

// Print the periodic "Starting event" line, honoring the log module and optional thread filter.
// The reported event number, count and rate are all per worker thread: each enabled thread logs
// every N events it processes, showing its own 1-based event count and average rate (events / second).
void GEventAction::log_event_start(int thread_id) {
	if (log_every_n <= 0) return;
	if (log_every_thread >= 0 && log_every_thread != thread_id) return;

	// Anchor this thread's clock on its first counted event, then count this event.
	const auto now = std::chrono::steady_clock::now();
	if (log_events_seen == 0) { log_start_time = now; }
	++log_events_seen;

	if (log_events_seen % log_every_n != 0) return;

	const double elapsed_s = std::chrono::duration<double>(now - log_start_time).count();
	const double rate      = elapsed_s > 0.0 ? static_cast<double>(log_events_seen) / elapsed_s : 0.0;

	// log_events_seen is this thread's own 1-based count, not the global Geant4 event id.
	log->info(0, "Starting event n. ", log_events_seen, " in thread ", thread_id,
	          ". Average rate: ", rate, " events / second");
}

// Begin-of-event hook used mainly for tracing event and thread identifiers.
void GEventAction::BeginOfEventAction([[maybe_unused]] const G4Event* event) {
	const auto thread_id = G4Threading::G4GetThreadId();
	const auto event_id  = event->GetEventID();

	log->debug(NORMAL, FUNCTION_NAME, " event id ", event_id, " in thread ", thread_id);
	if (track_provenance != nullptr) { track_provenance->clear(); }

	log_event_start(thread_id);
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

	const auto thread_id = G4Threading::G4GetThreadId();
	const auto event_id  = event->GetEventID();

	auto gevent_header       = std::make_unique<GEventHeader>(goptions, event_id, thread_id);
	auto eventDataCollection = std::make_shared<GEventDataCollection>(goptions, std::move(gevent_header));
	eventDataCollection->setGeneratedParticles(
		make_generated_particle_bank(GPrimaryGeneratorAction::currentGeneratedParticleRecords()));
	eventDataCollection->setGeneratedTrackedParticles(
		make_generated_particle_bank(GPrimaryGeneratorAction::currentGeneratedTrackedParticleRecords()));

	auto* const hcs_this_event = event->GetHCofThisEvent();
	if (hcs_this_event == nullptr) {
		if (!eventDataCollection->getGeneratedParticles().empty() ||
		    !eventDataCollection->getGeneratedTrackedParticles().empty()) {
			publish_event_data(eventDataCollection);
		}
		return;
	}

	const auto digi_map = run_action->get_digitization_routines_map();
	if (digi_map == nullptr) {
		log->error(ERR_GDIGIMAP_NOT_EXISTING, FUNCTION_NAME,
				   " no digitization routines map available in thread ", thread_id);
		return;
	}

	bool has_event_mode_payload = false;
	bool has_run_mode_payload   = false;
	const bool also_reject_true_info = scalar_bool_option_enabled(goptions, "also_reject_true_info");
	std::unordered_set<int> ancestor_track_ids;

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
		size_t accepted_hit_index = 0;

		// Process all hits in the collection. Event-mode digitizers append to the
		// event container, while run-mode digitizers append to the run container.
		for (size_t hitIndex = 0; hitIndex < this_ghc->GetSize(); ++hitIndex) {

			auto* const this_hit = static_cast<GHit*>(this_ghc->GetHit(hitIndex));
			if (this_hit == nullptr) {
				continue;
			}
			if (save_all_ancestors) {
				const auto track_ids = this_hit->getTids();
				ancestor_track_ids.insert(track_ids.begin(), track_ids.end());
			}

			auto digi_data = digitization_routine->digitizeHit(this_hit, hitIndex);
			bool hit_accepted = digi_data != nullptr;

			// Apply the post-digitization threshold and efficiency policies (-applyThresholds /
			// -applyInefficiencies). Both are evaluated (not short-circuited) so an enrolled
			// efficiency draw always happens. A skipped hit is dropped like a non-digitized one,
			// so the also_reject_true_info handling below suppresses its true-info row as well.
			if (hit_accepted) {
				const bool skip_threshold  = digitization_routine->apply_thresholds(this_hit, digi_data.get());
				const bool skip_efficiency = digitization_routine->apply_efficiency(this_hit, digi_data.get());
				if (skip_threshold || skip_efficiency) {
					digi_data.reset();
					hit_accepted = false;
				}
			}

			if (collection_mode == CollectionMode::event) {
				if (hit_accepted) {
					++accepted_hit_index;
					digi_data->includeVariable("hitn", static_cast<int>(accepted_hit_index));
					eventDataCollection->addDetectorDigitizedData(hcSDName, std::move(digi_data));
				}
				if (hit_accepted || !also_reject_true_info) {
					const size_t output_hit_index = hit_accepted ? accepted_hit_index : hitIndex + 1;
					auto true_data = digitization_routine->collectTrueInformation(this_hit, output_hit_index);
					if (save_original_track && track_provenance != nullptr && true_data != nullptr) {
						const int           tid = this_hit->getTid();
						const G4ThreeVector op  = track_provenance->originalTrackMomentum(tid);
						true_data->includeVariable("otid", track_provenance->originalTrackId(tid));
						true_data->includeVariable("opid", track_provenance->originalTrackPid(tid));
						true_data->includeVariable("opx", op.getX());
						true_data->includeVariable("opy", op.getY());
						true_data->includeVariable("opz", op.getZ());
					}
					eventDataCollection->addDetectorTrueInfoData(hcSDName, std::move(true_data));
					has_event_mode_payload = true;
				}
			}
			else if (collection_mode == CollectionMode::run) {
				if (hit_accepted) {
					run_action->collect_event_data_collections(
						hcSDName,
						std::move(digi_data));
					has_run_mode_payload = true;
				}
			}
		}
	}

	if (save_all_ancestors && track_provenance != nullptr) {
		eventDataCollection->setAncestors(
			make_ancestor_bank(track_provenance->ancestorsForTracks(ancestor_track_ids)));
	}

	// Record whether this event contributed at least one run-mode payload entry.
	if (has_run_mode_payload) {
		run_action->increment_run_events_with_payload();
	}

	// Publish event-mode output once, after all collections have been processed.
	if (has_event_mode_payload ||
	    !eventDataCollection->getAncestors().empty() ||
	    !eventDataCollection->getGeneratedParticles().empty() ||
	    !eventDataCollection->getGeneratedTrackedParticles().empty()) {
		publish_event_data(eventDataCollection);
	}
}

// Send the completed event-data object to every configured worker-thread streamer.
void GEventAction::publish_event_data(const std::shared_ptr<GEventDataCollection>& event_data) const {
	if (run_action == nullptr || event_data == nullptr) {
		return;
	}

	if (!run_action->has_streamer_threads_map()) {
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
