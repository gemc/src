// gemc
#include "gRunAction.h"
#include "gRun.h"
#include "../gactionConventions.h"
#include "gutsConventions.h"

// geant4
#include "G4Threading.hh"
#include "G4MTRunManager.hh"

std::mutex                   GRunAction::completed_run_data_mutex;
GRunAction::CompletedRunData GRunAction::completed_worker_run_data;


// Construct the run action and retain access to shared configuration and
// digitization services for the current execution context.
GRunAction::GRunAction(std::shared_ptr<GOptions>                           gopt,
                       std::shared_ptr<gdynamicdigitization::dRoutinesMap> digi_map) :
	GBase(gopt, GRUNACTION_LOGGER),
	goptions(std::move(gopt)),
	digitization_routines_map(std::move(digi_map)) {
	const auto desc = std::to_string(G4Threading::G4GetThreadId());
	log->debug(CONSTRUCTOR, FUNCTION_NAME, desc);
}


// Create the thread-local run object used by Geant4 for this execution context.
G4Run* GRunAction::GenerateRun() {
	log->debug(NORMAL, FUNCTION_NAME);

	return new GRun(goptions, digitization_routines_map);
}

// Initialize run-scoped bookkeeping, determine which streamer categories are
// needed for this run, and open the appropriate connections.
void GRunAction::BeginOfRunAction(const G4Run* aRun) {
	const auto thread_id = G4Threading::G4GetThreadId();
	const auto run       = aRun->GetRunID();

	auto run_header = std::make_unique<GRunHeader>(goptions, run, thread_id);
	run_data        = std::make_unique<GRunDataCollection>(goptions, std::move(run_header));

	const auto neventsThisRun = aRun->GetNumberOfEventToBeProcessed();

	// Reset the per-run mode flags before scanning the digitization routines.
	need_a_thread_streamer = false;
	need_a_run_streamer    = false;

	// Inspect the available digitization routines to determine whether this run
	// requires event-mode publication, run-mode publication, or both.
	if (digitization_routines_map != nullptr) {
		for (const auto& [plugin, digiRoutine] : *digitization_routines_map) {
			if (digiRoutine == nullptr) {
				log->error(ERR_GDIGIMAP_NOT_EXISTING, FUNCTION_NAME,
				           " null digitization routine registered for plugin ", plugin);
				continue;
			}

			if (digiRoutine->collection_mode() == CollectionMode::event) {
				need_a_thread_streamer = true;
			}
			else if (digiRoutine->collection_mode() == CollectionMode::run) {
				need_a_run_streamer = true;
			}
		}
	}
	else {
		log->error(ERR_GDIGIMAP_NOT_EXISTING, FUNCTION_NAME,
		           " digitization_routines_map is null - streamer mode detection skipped.");
	}

	// Worker threads own event-mode publication, so they lazily build and open
	// thread-local streamers only when at least one event-mode digitizer exists.
	if (!IsMaster() && need_a_thread_streamer) {
		if (gstreamer_threads_map == nullptr) {
			log->info(1, "Defining thread gstreamers for run ", run, " in thread ", thread_id);
			gstreamer_threads_map = gstreamer::gstreamersMapPtr(goptions, thread_id);
		}

		if (gstreamer_threads_map == nullptr) {
			log->error(1, FUNCTION_NAME, " gstreamer_threads_map is null in thread ", thread_id,
			           " - cannot open connections.");
			return;
		}

		for (const auto& [name, gstreamer] : *gstreamer_threads_map) {
			if (gstreamer == nullptr) {
				log->error(ERR_STREAMERMAP_NOT_EXISTING,
				           "Null GStreamer entry ", name, " in thread ", thread_id);
				continue;
			}

			if (!gstreamer->openConnection()) {
				log->error(ERR_STREAMERMAP_NOT_EXISTING,
				           "Failed to open connection for GStreamer ", name,
				           " in thread ", thread_id);
			}

			log->info(2, FUNCTION_NAME, "Worker thread [", thread_id, "]: opening connection for ",
			          KGRN, name, RST,
			          " for run ", run, ". Number of events to be processed: ", neventsThisRun);
		}
	}
	// The master thread owns run-mode publication, so it opens the run streamers
	// only when at least one digitizer accumulates payload at run scope.
	else if (IsMaster() && need_a_run_streamer) {
		if (gstreamer_run_map == nullptr) {
			log->info(1, "Defining run gstreamers for run ", run);
			gstreamer_run_map = gstreamer::gstreamersMapPtr(goptions);
		}

		if (gstreamer_run_map == nullptr) {
			log->error(1, FUNCTION_NAME, " gstreamer_run_map is null in master thread ",
			           " - cannot open connections.");
			return;
		}

		for (const auto& [name, gstreamer] : *gstreamer_run_map) {
			if (gstreamer == nullptr) {
				log->error(ERR_STREAMERMAP_NOT_EXISTING,
				           "Null GStreamer entry ", name, " in master thread");
				continue;
			}

			if (!gstreamer->openConnection()) {
				log->error(ERR_STREAMERMAP_NOT_EXISTING,
				           "Failed to open connection for GStreamer in master thread ", name);
			}

			log->info(2, FUNCTION_NAME, "Master Thread: opening connection for ",
			          KGRN, name, RST,
			          " for run ", run, ". Number of events to be processed: ", neventsThisRun);
		}
	}
}

// Close streamers at run end and, when running on the master thread, merge and
// publish the run-level payload accumulated by workers.
void GRunAction::EndOfRunAction(const G4Run* aRun) {
	const auto        thread_id = G4Threading::G4GetThreadId();
	const auto        run       = aRun->GetRunID();
	const std::string what_am_i = IsMaster() ? "Master" : "Worker";

	if (!IsMaster() && need_a_thread_streamer) {
		if (gstreamer_threads_map == nullptr) {
			log->error(ERR_STREAMERMAP_NOT_EXISTING, FUNCTION_NAME,
			           " gstreamer_map is null in thread ", thread_id,
			           " - cannot close connections.");
		}
		else {
			for (const auto& [name, gstreamer] : *gstreamer_threads_map) {
				log->info(2, FUNCTION_NAME, " ", what_am_i, " [", thread_id, "], for run ", run,
				          " closing connection for gstreamer ", name);

				if (gstreamer == nullptr) {
					log->error(ERR_STREAMERMAP_NOT_EXISTING,
					           "Null GStreamer entry ", name, " in thread ", thread_id);
					continue;
				}

				if (!gstreamer->closeConnection()) {
					log->error(1, "Failed to close connection for GStreamer ", name, " in thread ", thread_id);
				}
			}
		}
	}

	// Worker threads do not publish merged run data. Instead, they hand their
	// completed run-level accumulation to the shared pool and return.
	if (!IsMaster()) {
		stash_worker_run_data();
		return;
	}

	if (IsMaster() && need_a_run_streamer) {
		// Gather all worker-produced run data for this run and merge them into a
		// single master-side run-data object before publication.
		auto completed_run_data = take_completed_worker_run_data();
		log->info(2, FUNCTION_NAME,
		          " master collected ", static_cast<int>(completed_run_data.size()),
		          " worker run_data object(s) for run ", run);

		std::shared_ptr<GRunDataCollection> merged_run_data;

		for (auto& worker_run_data : completed_run_data) {
			if (worker_run_data == nullptr) {
				continue;
			}

			// Create the merged destination lazily only if there is at least one
			// valid worker contribution to merge.
			if (merged_run_data == nullptr) {
				auto merged_header = std::make_unique<GRunHeader>(goptions, run, thread_id);
				merged_run_data    = std::make_shared<GRunDataCollection>(goptions, std::move(merged_header));
			}

			merged_run_data->merge(*worker_run_data);
		}

		// Publish the merged run-level payload once, after all workers have contributed.
		if (merged_run_data != nullptr) {
			publish_run_data(merged_run_data);
		}

		if (gstreamer_run_map == nullptr) {
			log->error(ERR_STREAMERMAP_NOT_EXISTING, FUNCTION_NAME,
			           " gstreamer_map is null in master thread - cannot close connections.");
			return;
		}

		for (const auto& [name, gstreamer] : *gstreamer_run_map) {
			log->info(2, FUNCTION_NAME, " ", what_am_i, " for run ", run,
			          " closing connection for gstreamer ", name);

			if (gstreamer == nullptr) {
				log->error(ERR_STREAMERMAP_NOT_EXISTING,
				           "Null GStreamer entry ", name, " in master thread");
				continue;
			}

			if (!gstreamer->closeConnection()) {
				log->error(1, "Failed to close connection for GStreamer ", name, " in master thread");
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
void GRunAction::publish_run_data(const std::shared_ptr<GRunDataCollection>& run_data) const {
	if (run_data == nullptr) {
		log->error(ERR_GRUNACTION_NOT_EXISTING, FUNCTION_NAME,
		           " run_data is null - cannot publish merged run data.");
		return;
	}

	if (gstreamer_run_map == nullptr) {
		log->error(ERR_STREAMERMAP_NOT_EXISTING, FUNCTION_NAME,
		           " no run streamer map available - run data will not be published.");
		return;
	}

	for (const auto& [name, gstreamer] : *gstreamer_run_map) {
		if (gstreamer == nullptr) {
			log->error(ERR_STREAMERMAP_NOT_EXISTING, FUNCTION_NAME,
			           " null gstreamer instance for run streamer ", name);
			continue;
		}

		gstreamer->publishRunData(run_data);
	}
}

// (Legacy/experimental streaming logic remains commented out below.)

// TODO: 2 more is too much we need some calculation here
// int nFramesToCreate = neventsThisRun * eventDuration / frameDuration + 2;

// if (stream) {
// 	if (frameStreamVerbosity >= GVERBOSITY_SUMMARY) {
// 		cout << SROLOGHEADER << " current nframes in the buffer: " << frameRunData.size() << ", new frames to create: " << nFramesToCreate;
// 		cout << ", last frame id created: " << lastFrameCreated << endl;
// 	}
//
// 	for (int f = lastFrameCreated; f < lastFrameCreated + nFramesToCreate; f++) {
// 		GFrameDataCollectionHeader* gframeHeader = new GFrameDataCollectionHeader(f + 1, frameDuration, verbosity);
// 		GFrameDataCollection*       frameData    = new GFrameDataCollection(gframeHeader, verbosity);
// 		frameRunData.push_back(frameData);
// 	}
//
// 	lastFrameCreated += nFramesToCreate;
// 	if (frameStreamVerbosity >= GVERBOSITY_SUMMARY) {
// 		cout << SROLOGHEADER << nFramesToCreate << " new frames, buffer size is now " << frameRunData.size();
// 		cout << ", last frame id created: " << lastFrameCreated << endl;
// 	}
// }


// looping over run data and filling frameRunData
// need to remember last event number here
// if (stream) {
// 	for (auto eventDataCollection : theRun->getRunData()) {
// 		int absoluteEventNumber = eventIndex + eventDataCollection->getEventNumber();
//
// 		// filling frameRunData with this eventDataCollection
// 		for (auto [detectorName, gdataCollection] : *eventDataCollection->getDataCollectionMap()) {
// 			for (auto hitDigitizedData : *gdataCollection->getDigitizedData()) {
// 				int timeAtelectronic = hitDigitizedData->getTimeAtElectronics();
// 				if (timeAtelectronic != TIMEATELECTRONICSNOTDEFINED) {
// 					int frameIndex = eventFrameIndex(absoluteEventNumber, timeAtelectronic);
// 					frameRunData[frameIndex]->addIntegralPayload(formPayload(hitDigitizedData), verbosity);
// 				}
// 			}
// 		}
// 	}
// }


// now flushing all frames past eventIndex

// if (stream) {
// 	// updating eventIndex
// 	eventIndex += neventsThisRun;
//
// 	for (auto [factoryName, streamerFactory] : *gstreamerFactoryMap) {
// 		if (streamerFactory->getStreamType() == "stream" && frameRunData.size() > 0) {
// 			// need to look for additional frame to flush
// 			int nFramesToFlush = nFramesToCreate - 2;
//
// 			if (frameStreamVerbosity >= GVERBOSITY_SUMMARY) { cout << SROLOGHEADER << "number of frames to flush: " << nFramesToFlush << endl; }
// 			for (auto fid = 0; fid < nFramesToFlush; fid++) {
// 				logSummary("Streaming frame id <" + to_string(frameRunData.front()->getFrameID()) + " using streamer factory >" + factoryName + "<");
// 				streamerFactory->publishFrameRunData(goptions, frameRunData.front());
// 				delete frameRunData.front();
// 				frameRunData.erase(frameRunData.begin());
// 			}
// 		}
// 	}
// }


// determine the frame ID based on event number, eventDuration, frameDuration and number of threads
// add frameData to frameRunData if it's not present
// int GRunAction::eventFrameIndex(int eventNumber, double timeAtElectronics) {
// 	int absoluteHitTime = eventNumber * eventDuration + timeAtElectronics;
// 	int frameID         = absoluteHitTime / frameDuration + 1;
// 	int frameIndex      = -1;
//
// 	// cout << "eventNumber: " << eventNumber << ", absoluteHitTime: " << absoluteHitTime << ", frameID: " << frameID << endl;
//
// 	for (size_t f = 0; f < frameRunData.size(); f++) { if (frameRunData[f]->getFrameID() == frameID) { frameIndex = (int)f; } }
// 	// cout << "eventNumber: " << eventNumber << ", absoluteHitTime: " << absoluteHitTime << ", frameIndex: " << frameIndex << endl;
//
// 	return frameIndex;
// }

// vector<int> GRunAction::formPayload(GDigitizedData* digitizedData) {
// 	vector<int> payload;
//
// 	int crate   = digitizedData->getIntObservable(CRATESTRINGID);
// 	int slot    = digitizedData->getIntObservable(SLOTSTRINGID);
// 	int channel = digitizedData->getIntObservable(CHANNELSTRINGID);
// 	int q       = digitizedData->getIntObservable(CHARGEATELECTRONICS);
// 	int time    = digitizedData->getIntObservable(TIMEATELECTRONICS);
//
// 	payload.push_back(crate);
// 	payload.push_back(slot);
// 	payload.push_back(channel);
// 	payload.push_back(q);
// 	payload.push_back(time);
//
// 	return payload;
// }
//
// bool GRunAction::findFrameID(int fid) {
// 	for (auto frame : frameRunData) { if (frame->getFrameID() == fid) { return true; } }
// 	return false;
// }