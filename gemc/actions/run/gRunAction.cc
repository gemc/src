// gemc
#include "gRunAction.h"
#include "gRun.h"
#include "../gactionConventions.h"
#include "gutsConventions.h"

// geant4
#include "G4Threading.hh"

std::mutex GRunAction::completed_run_data_mutex;
GRunAction::CompletedRunData GRunAction::completed_worker_run_data;


// Construct the run action and retain access to shared configuration and
// digitization services for the current execution context.
GRunAction::GRunAction(std::shared_ptr<GOptions> gopt,
                       std::shared_ptr<gdynamicdigitization::dRoutinesMap> digi_map,
                       std::shared_ptr<GAnalysisAccumulator> analyzer,
                       std::shared_ptr<GSROFactory> sro) : GBase(gopt, GRUNACTION_LOGGER),
	goptions(std::move(gopt)),
	digitization_routines_map(std::move(digi_map)),
	analysis_accumulator(std::move(analyzer)),
	sro_factory(std::move(sro)) {
	const auto desc = std::to_string(G4Threading::G4GetThreadId());
	log->debug(CONSTRUCTOR, "%1%2", FUNCTION_NAME, desc);
}


// Create the thread-local run object used by Geant4 for this execution context.
G4Run *GRunAction::GenerateRun() {
	log->debug(NORMAL, "%1", FUNCTION_NAME);

	return new GRun(goptions, digitization_routines_map);
}

// Initialize run-scoped bookkeeping, determine which streamer categories are
// needed for this run, and open the appropriate connections.
void GRunAction::BeginOfRunAction(const G4Run *aRun) {
	const auto thread_id = G4Threading::G4GetThreadId();
	const auto run = aRun->GetRunID();

	auto run_header = std::make_unique<GRunHeader>(goptions, run, thread_id);
	run_data = std::make_unique<GRunDataCollection>(goptions, std::move(run_header));
	if (analysis_accumulator != nullptr) {
		analysis_run_number = analysis_accumulator->currentRunNumber();
		analysis_shard = std::make_unique<GAnalysisShard>();
	}

	const auto neventsThisRun = aRun->GetNumberOfEventToBeProcessed();
	if (sro_factory && IsMaster()) {
		try { sro_factory->begin_run(run, static_cast<GSROEventId>(neventsThisRun)); }
		catch (const std::exception& error) {
			log->error(gstreamer::ERR_PUBLISH_ERROR, "SRO begin run: %1", error.what());
		}
	}

	// Reset the per-run mode flags before scanning the digitization routines.
	need_a_thread_streamer = false;
	need_a_run_streamer = false;

	// Inspect the available digitization routines to determine whether this run
	// requires event-mode publication, run-mode publication, or both.
	if (digitization_routines_map != nullptr) {
		for (const auto &[plugin, digiRoutine]: *digitization_routines_map) {
			if (digiRoutine == nullptr) {
				log->error(
					gaction::ERR_GDIGIMAP_NOT_EXISTING, "%1 null digitization routine registered for plugin %2",
					FUNCTION_NAME, plugin);
			}

			if (digiRoutine->collection_mode() == CollectionMode::event) {
				need_a_thread_streamer = true;
			} else if (digiRoutine->collection_mode() == CollectionMode::run) {
				to_normalize[plugin] = digiRoutine->variables_to_normalize();
				need_a_run_streamer = true;
			}
		}
	} else {
		log->error(
			gaction::ERR_GDIGIMAP_NOT_EXISTING,
			"%1 digitization_routines_map is null - streamer mode detection skipped.", FUNCTION_NAME);
	}

	for (const auto& streamer_definition : gstreamer::getGStreamerDefinition(goptions)) {
		if (streamer_definition.type == "event") {
			need_a_thread_streamer = true;
			break;
		}
	}


	// Worker threads own event-mode publication, so they lazily build and open
	// thread-local streamers only when at least one event-mode digitizer exists.
	if (!IsMaster() && need_a_thread_streamer) {
		if (gstreamer_threads_map == nullptr) {
			log->info(1, "Defining thread gstreamers for run %1 in thread %2", run, thread_id);
			gstreamer_threads_map = gstreamer::gstreamersMapPtr(goptions, thread_id);
		}

		if (gstreamer_threads_map == nullptr) {
			log->error(
				1, "%1 gstreamer_threads_map is null in thread %2 - cannot open connections.", FUNCTION_NAME,
				thread_id);
		}

		for (const auto &[name, gstreamer]: *gstreamer_threads_map) {
			if (gstreamer == nullptr) {
				log->error(
					gaction::ERR_STREAMERMAP_NOT_EXISTING, "Null GStreamer entry %1 in thread %2", name,
					thread_id);
			}

			if (!gstreamer->openConnection()) {
				log->error(
					gaction::ERR_STREAMERMAP_NOT_EXISTING,
					"Failed to open connection for GStreamer %1 in thread %2", name, thread_id);
			}

			log->info(
				2,
				"%1Worker thread [%2]: opening connection for %3%4%5 for run %6. Number of events to be "
				"processed: %7", FUNCTION_NAME, thread_id, guts::KGRN, name, guts::RST, run, neventsThisRun);
		}
	}
	// The master thread owns run-mode publication, so it opens the run streamers
	// only when at least one digitizer accumulates payload at run scope.
	else if (IsMaster() && need_a_run_streamer) {
		if (gstreamer_run_map == nullptr) {
			log->info(1, "Defining run gstreamers for run %1", run);
			gstreamer_run_map = gstreamer::gstreamersMapPtr(goptions);
		}

		if (gstreamer_run_map == nullptr) {
			log->error(
				1, "%1 gstreamer_run_map is null in master thread  - cannot open connections.", FUNCTION_NAME);
		}

		for (const auto &[name, gstreamer]: *gstreamer_run_map) {
			if (gstreamer == nullptr) {
				log->error(
					gaction::ERR_STREAMERMAP_NOT_EXISTING, "Null GStreamer entry %1 in master thread", name);
			}

			if (!gstreamer->openConnection()) {
				log->error(
					gaction::ERR_STREAMERMAP_NOT_EXISTING,
					"Failed to open connection for GStreamer in master thread %1", name);
			}

			log->info(
				2,
				"%1Master Thread: opening connection for %2%3%4 for run %5. Number of events to be "
				"processed: %6", FUNCTION_NAME, guts::KGRN, name, guts::RST, run, neventsThisRun);
		}
	}
}

// Close streamers at run end and, when running on the master thread, merge and
// publish the run-level payload accumulated by workers.
void GRunAction::EndOfRunAction(const G4Run *aRun) {
	const auto thread_id = G4Threading::G4GetThreadId();
	const auto runNumber = aRun->GetRunID();
	const std::string what_am_i = IsMaster() ? "Master" : "Worker";
	// In MT Geant4 reaches the master here after worker run actions; sequential mode owns both roles.
	if (sro_factory && IsMaster()) {
		try { sro_factory->finish_run(aRun->GetNumberOfEvent() == aRun->GetNumberOfEventToBeProcessed()); }
		catch (const std::exception& error) {
			// finish_run has already joined all crate/progress threads before reporting the first error.
			log->error(gstreamer::ERR_PUBLISH_ERROR, "SRO finish run: %1", error.what());
		}
	}

	if (!IsMaster() && need_a_thread_streamer) {
		if (gstreamer_threads_map == nullptr) {
			log->error(
				gaction::ERR_STREAMERMAP_NOT_EXISTING,
				"%1 gstreamer_map is null in thread %2 - cannot close connections.", FUNCTION_NAME, thread_id);
		} else {
			for (const auto &[name, gstreamer]: *gstreamer_threads_map) {
				log->info(
					2, "%1 %2 [%3], for run %4 closing connection for gstreamer %5", FUNCTION_NAME, what_am_i,
					thread_id, runNumber, name);

				if (gstreamer == nullptr) {
					log->error(
						gaction::ERR_STREAMERMAP_NOT_EXISTING, "Null GStreamer entry %1 in thread %2", name,
						thread_id);
				}

				if (!gstreamer->closeConnection()) {
					log->error(1, "Failed to close connection for GStreamer %1 in thread %2", name, thread_id);
				}
			}
		}
	}

	// Each execution context writes only to its private shard. The shared accumulator locks once here.
	if (analysis_accumulator != nullptr && analysis_shard != nullptr) {
		if (!analysis_shard->empty()) { analysis_accumulator->merge(std::move(*analysis_shard)); }
		analysis_shard.reset();
	}

	// Worker threads do not publish merged run data. Instead, they hand their
	// completed run-level accumulation to the shared pool and return.
	if (!IsMaster()) {
		// Only contribute to the master merge pool when a run-mode streamer will drain it;
		// otherwise the pool is never taken and would accumulate stale prior-run data.
		if (need_a_run_streamer) { stash_worker_run_data(); }
		return;
	}

	if (IsMaster() && need_a_run_streamer) {
		// Gather all worker-produced run data for this run and merge them into a
		// single master-side run-data object before publication.
		auto completed_run_data = take_completed_worker_run_data();
		log->info(
			2, "%1 master collected %2 worker run_data object(s) for run %3", FUNCTION_NAME,
			static_cast<int>(completed_run_data.size()), runNumber);

		std::shared_ptr<GRunDataCollection> merged_run_data;

		for (auto &worker_run_data: completed_run_data) {
			if (worker_run_data == nullptr) {
				continue;
			}

			// Create the merged destination lazily only if there is at least one
			// valid worker contribution to merge.
			if (merged_run_data == nullptr) {
				auto merged_header = std::make_unique<GRunHeader>(goptions, runNumber, thread_id);
				merged_run_data = std::make_shared<GRunDataCollection>(goptions, std::move(merged_header));
			}

			merged_run_data->merge(*worker_run_data);
		}

		// Publish the merged run-level payload once, after all workers have contributed.
		if (merged_run_data != nullptr) {
			publish_run_data(merged_run_data);
		}

		if (gstreamer_run_map == nullptr) {
			log->error(
				gaction::ERR_STREAMERMAP_NOT_EXISTING,
				"%1 gstreamer_map is null in master thread - cannot close connections.", FUNCTION_NAME);
		}

		for (const auto &[name, gstreamer]: *gstreamer_run_map) {
			log->info(
				2, "%1 %2 for run %3 closing connection for gstreamer %4", FUNCTION_NAME, what_am_i, runNumber,
				name);

			if (gstreamer == nullptr) {
				log->error(
					gaction::ERR_STREAMERMAP_NOT_EXISTING, "Null GStreamer entry %1 in master thread", name);
			}

			if (!gstreamer->closeConnection()) {
				log->error(1, "Failed to close connection for GStreamer %1 in master thread", name);
			}
		}
	}

}

// Move this worker's completed run-data object into the protected static pool
// so it can later be collected by the master thread.
void GRunAction::stash_worker_run_data() {
	if (run_data == nullptr) {
		return;
	}

	std::scoped_lock lock(completed_run_data_mutex);
	completed_worker_run_data.emplace_back(std::move(run_data));
}

// Extract and clear the protected pool of completed worker run-data objects.
auto GRunAction::take_completed_worker_run_data() -> CompletedRunData {
	std::scoped_lock lock(completed_run_data_mutex);

	auto result = std::move(completed_worker_run_data);
	completed_worker_run_data.clear();

	return result;
}


// Publish the merged run-level payload to every configured master-side run streamer.
void GRunAction::publish_run_data(const std::shared_ptr<GRunDataCollection> &run_data_collaction) const {
	if (run_data_collaction == nullptr) {
		log->error(
			gaction::ERR_GRUNACTION_NOT_EXISTING, "%1 run_data is null - cannot publish merged run data.",
			FUNCTION_NAME);
	}

	if (gstreamer_run_map == nullptr) {
		log->error(
			gaction::ERR_STREAMERMAP_NOT_EXISTING,
			"%1 no run streamer map available - run data will not be published.", FUNCTION_NAME);
	}

	// Normalize once, before publishing: normalize_run_data() mutates run_data_collaction
	// in place and is NOT idempotent, so running it per streamer would divide the run
	// observables by events_processed once for every configured run streamer.
	normalize_run_data(run_data_collaction);

	for (const auto &[name, gstreamer]: *gstreamer_run_map) {
		if (gstreamer == nullptr) {
			log->error(
				gaction::ERR_STREAMERMAP_NOT_EXISTING, "%1 null gstreamer instance for run streamer %2",
				FUNCTION_NAME, name);
		}

		gstreamer->publishRunData(run_data_collaction);
	}
}


void GRunAction::normalize_run_data(const std::shared_ptr<GRunDataCollection> &run_data_collaction) const {
	if (run_data_collaction == nullptr) {
		log->error(
			gaction::ERR_GRUNACTION_NOT_EXISTING, "%1 run_data_collaction is null - cannot normalize run data.",
			FUNCTION_NAME);
	}

	const int events_processed = run_data_collaction->get_events_processed();
	if (events_processed <= 0) {
		log->warning("%1 events_processed is %2 - skipping normalization.", FUNCTION_NAME, events_processed);
		return;
	}

	const double norm = static_cast<double>(events_processed);

	for (auto &[sdName, dataCollection]: run_data_collaction->getMutableDataCollectionMap()) {
		if (dataCollection == nullptr) {
			log->warning("%1 detector %2 has null data collection - skipping.", FUNCTION_NAME, sdName);
			continue;
		}

		auto &digitizedData = dataCollection->getMutableDigitizedData();
		if (digitizedData.empty() || digitizedData.front() == nullptr) {
			continue;
		}

		auto &digitized = digitizedData.front();
		// we are in a const method so we can't loop directky over to_normalize[sdName]
		// cause this call could modify the map!

		auto it = to_normalize.find(sdName);
		if (it != to_normalize.end()) {
			for (const auto &varName: it->second) {
				const auto intVars = digitized->getIntObservablesMap(0);
				const auto intIt = intVars.find(varName);
				if (intIt != intVars.end()) {
					digitized->includeVariable(varName, static_cast<double>(intIt->second) / norm);
					continue;
				}

				const auto dblVars = digitized->getDblObservablesMap(0);
				const auto dblIt = dblVars.find(varName);
				if (dblIt != dblVars.end()) {
					digitized->includeVariable(varName, dblIt->second / norm);
				}
			}
		}
	}
}
