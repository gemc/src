#pragma once

#include <memory>
#include <mutex>
#include <vector>

// geant4
#include "G4UserRunAction.hh"

// gemc
#include "gbase.h"
#include "gdynamicdigitization.h"
#include "gstreamer.h"
#include "run/gRunDataCollection.h"
#include "actions/gactionConventions.h"


/**
 * @file gRunAction.h
 * @brief Declares GRunAction, responsible for run lifecycle hooks and run object creation.
 *
 * @ingroup gactions_module
 */

constexpr const char* GRUNACTION_LOGGER = "grunaction";

/**
 * @brief Namespace collecting helpers for the run action.
 *
 * @ingroup gactions_module
 */
namespace grunaction {
	/**
	 * @brief Returns the options associated with the run action.
	 *
	 * @return A GOptions instance for the run action logger scope.
	 */
	inline GOptions defineOptions() { return GOptions(GRUNACTION_LOGGER); }
} // namespace grunaction


/**
 * @class GRunAction
 * @brief Handles run begin/end callbacks and creates the per-thread run object.
 *
 * Responsibilities:
 * - Create the per-thread run object (GRun) via GenerateRun().
 * - At the start of each run on worker threads, create the per-thread streamer map
 *   (once) and open streamer connections for the run.
 * - At the end of each run on worker threads, close streamer connections.
 * - Accumulate run-level data on each worker thread and expose a master-side
 *   aggregation path at end of run.
 *
 * Threading:
 * - Worker threads own their local streamer map instance.
 * - The master thread owns the run-level streamer map when run-mode digitizers exist.
 * - Worker-produced run data are collected into a process-wide protected pool,
 *   then accessed by the master in EndOfRunAction().
 *
 * @ingroup gactions_module
 */
class GRunAction : public GBase<GRunAction>, public G4UserRunAction
{
public:
	/**
	 * @brief Constructs the run action.
	 *
	 * @param gopts Shared configuration used by the run action and by the created run object.
	 * @param digi_map Shared digitization routines map to be passed into the created run object.
	 */
	explicit GRunAction(std::shared_ptr<GOptions>                           gopts,
						std::shared_ptr<gdynamicdigitization::dRoutinesMap> digi_map);

	~GRunAction() override = default;

	// Let's make it clear: GRunAction is not copyable or moveable
	GRunAction(const GRunAction&)            = delete;
	GRunAction& operator=(const GRunAction&) = delete;
	GRunAction(GRunAction&&)                 = delete;
	GRunAction& operator=(GRunAction&&)      = delete;

	/**
	 * @brief Returns the shared digitization routines map.
	 *
	 * This map is used by event-level logic to select the correct digitization routine
	 * for each hit collection.
	 *
	 * @return Shared pointer to the digitization routines map.
	 */
	[[nodiscard]] auto get_digitization_routines_map() const
		-> std::shared_ptr<gdynamicdigitization::dRoutinesMap> {
		return digitization_routines_map;
	}

	/**
	 * @brief Returns the per-thread streamer map if it has been created.
	 *
	 * The map is instantiated lazily for worker threads at the beginning of a run.
	 *
	 * @return Shared pointer to the streamer map, or nullptr if not available.
	 */
	[[nodiscard]] auto get_streamer_threads_map() const
		-> std::shared_ptr<const gstreamer::gstreamersMap> {
		if (!gstreamer_threads_map) {
			log->error(ERR_STREAMERMAP_NOT_EXISTING, FUNCTION_NAME, " no gstreamer thread map available");
		}
		return gstreamer_threads_map;
	}

	/**
	 * @brief Adds one digitized/truth payload pair to the current thread run-level collection.
	 *
	 * @param hcSDName Hit collection / sensitive detector name.
	 * @param digi_data Digitized payload for one hit.
	 */
	void collect_event_data_collections(const std::string&              hcSDName,
										std::unique_ptr<GDigitizedData> digi_data) {
		if (run_data == nullptr) {
			log->error(ERR_GRUNACTION_NOT_EXISTING, FUNCTION_NAME,
					   " run_data is null - cannot collect run-level payload for collection ", hcSDName);
		}

		run_data->collect_event_data_collections(
			hcSDName,
			std::move(digi_data));
	}

	void increment_run_events_processed() {
		if (run_data == nullptr) {
			log->error(ERR_GRUNACTION_NOT_EXISTING, FUNCTION_NAME,
					   " run_data is null - cannot increment processed events.");
			return;
		}

		auto& header = run_data->getHeader();
		if (header == nullptr) {
			log->error(ERR_GRUNACTION_NOT_EXISTING, FUNCTION_NAME,
					   " run_data header is null - cannot increment processed events.");
			return;
		}

		header->increment_events_processed();
	}

	void increment_run_events_with_payload() {
		if (run_data == nullptr) {
			log->error(ERR_GRUNACTION_NOT_EXISTING, FUNCTION_NAME,
					   " run_data is null - cannot increment payload events.");
			return;
		}

		auto& header = run_data->getHeader();
		if (header == nullptr) {
			log->error(ERR_GRUNACTION_NOT_EXISTING, FUNCTION_NAME,
					   " run_data header is null - cannot increment payload events.");
			return;
		}

		header->increment_events_with_payload();
	}

private:
	using CompletedRunData = std::vector<std::unique_ptr<GRunDataCollection>>;

	/**
	 * @brief Moves the current worker-thread run data into the global completed-run pool.
	 *
	 * This is used to collect run-level data produced on worker threads so that
	 * the master thread can inspect them at the end of the run.
	 */
	void stash_worker_run_data();

	/**
	 * @brief Moves all completed worker run-data objects out of the global pool.
	 *
	 * This method is intended to be called by the master thread at the end of the run.
	 *
	 * @return A vector containing all worker-produced run data accumulated so far.
	 */
	[[nodiscard]] CompletedRunData take_completed_worker_run_data();

	/**
	 * @brief Creates and returns the run object for the current thread.
	 *
	 * This is invoked by Geant4 after BeamOn() begins and is used to create the
	 * thread-local GRun instance.
	 *
	 * @return Newly allocated run object for the current thread.
	 */
	G4Run* GenerateRun() override;

	/**
	 * @brief Called by Geant4 at the beginning of a run.
	 *
	 * Worker-thread behavior:
	 * - Lazily instantiate the streamer map (once).
	 * - Open each streamer connection for the run.
	 *
	 * Master-thread behavior:
	 * - Lazily instantiate the run streamer map (once), if needed.
	 *
	 * @param run The Geant4 run descriptor for the current run.
	 */
	void BeginOfRunAction(const G4Run* run) override;

	/**
	 * @brief Called by Geant4 at the end of a run.
	 *
	 * Worker-thread behavior:
	 * - Close each streamer connection for the run.
	 * - Transfer per-thread run data into the protected completed-run pool.
	 *
	 * Master-thread behavior:
	 * - Close each master run streamer connection for the run.
	 * - Collect all worker run data accumulated during the run.
	 *
	 * @param run The Geant4 run descriptor for the current run.
	 */
	void EndOfRunAction(const G4Run* run) override;

	/**
	 * @brief Shared configuration used by this run action and passed to dependent components.
	 */
	std::shared_ptr<GOptions> goptions;

	/**
	 * @brief Digitization routines map used by run/event actions to digitize hit collections.
	 */
	std::shared_ptr<gdynamicdigitization::dRoutinesMap> digitization_routines_map;

	/**
	 * @brief Per-thread streamer map (worker threads only), instantiated at run start.
	 */
	std::shared_ptr<const gstreamer::gstreamersMap> gstreamer_threads_map;

	/**
	 * @brief Run streamer map (master thread only), instantiated at run start.
	 */
	std::shared_ptr<const gstreamer::gstreamersMap> gstreamer_run_map;

	/**
	 * @brief Per-thread run-level data accumulated from event threads for run-mode digitizers.
	 */
	std::unique_ptr<GRunDataCollection> run_data;

	/**
	 * @brief True if at least one digitizer requires event-mode streaming.
	 */
	bool need_a_thread_streamer = false;

	/**
	 * @brief True if at least one digitizer requires run-mode streaming.
	 */
	bool need_a_run_streamer = false;

	/**
	 * @brief Process-wide pool of worker-completed run data.
	 */
	static std::mutex completed_run_data_mutex;

	/**
	 * @brief Process-wide storage for worker-completed run data awaiting master collection.
	 * The lock mechanism ensure the workers accumulation is complete
	 */
	static CompletedRunData completed_worker_run_data;

	void publish_run_data(const std::shared_ptr<GRunDataCollection>& run_data) const;
};


// vector of frame data in the run (local thread, merged in GRun::Merge in the global thread)
//	std::vector<GFrameDataCollection*> frameRunData;

//
// double frameDuration    = UNINITIALIZEDNUMBERQUANTITY; // frame length in nanoseconds
// double eventDuration    = UNINITIALIZEDNUMBERQUANTITY; // event duration in nanoseconds
// int    eventIndex       = 0;                           // added to the absolute event number, increases with each run
// int    lastFrameCreated = 0;                           // keeping track of the last frame created
//
// // determine the frame ID based on event number, eventDuration, frameDuration
// // TODO: this should be in gdata?
// int  eventFrameIndex(int eventNumber, double timeAtElectronics);
// bool findFrameID(int fid);
//
// // decide whether to write or not to stream the frame based on event number, eventDuration, frameDuration and number of threads
// // streaming the frame also deletes it from frameRunData
// // TODO: this should be in gdata?
// bool shouldWriteFrameID(int eventNumber, long int frameID);
//
// // TODO: this should be in gdata?
// std::vector<int> formPayload(GDigitizedData* digitizedData);


// in the constructur we had:

// frameDuration = 64000;
// eventDuration = gutilities::getG4Number(goptions->getScalarString("eventTimeSize"));

// stream = gopt->getSwitch("stream");