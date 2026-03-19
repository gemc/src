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
 * \file gRunAction.h
 * \brief Declares GRunAction, the run-lifecycle action for the GEMC actions module.
 *
 * @ingroup gactions_module
 */

constexpr const char* GRUNACTION_LOGGER = "grunaction";

/**
 * \brief Namespace containing helpers related to run-action configuration.
 *
 * @ingroup gactions_module
 */
namespace grunaction {
	/**
	 * \brief Returns the options associated with the run-action logger scope.
	 *
	 * \return A GOptions object scoped to the run-action logger name.
	 */
	inline GOptions defineOptions() { return GOptions(GRUNACTION_LOGGER); }
} // namespace grunaction


/**
 * @class GRunAction
 * \brief Handles run begin/end callbacks and creates the thread-local GRun object.
 *
 * This class is the GEMC implementation of the Geant4 run action. It participates
 * in both worker-thread and master-thread execution.
 *
 * Its main responsibilities are:
 * - create the per-thread GRun instance through
 *   the private method \c GenerateRun();
 * - initialize run-scoped bookkeeping at the start of a run;
 * - determine whether event-mode and/or run-mode streamers are needed;
 * - open and close the appropriate streamer connections;
 * - accumulate run-mode data on worker threads;
 * - gather and publish merged run-mode data on the master thread.
 *
 * Threading model:
 * - Worker threads can own thread streamer maps used for event publication.
 * - The master thread can own a run streamer map used for merged run publication.
 * - Worker-produced run data are moved into a protected static pool and consumed
 *   later by the master thread at run end.
 *
 * @ingroup gactions_module
 */
class GRunAction : public GBase<GRunAction>, public G4UserRunAction
{
public:
	/**
	 * \brief Constructs the run action.
	 *
	 * \param gopts Shared configuration object used by the run action and the GRun object it creates.
	 * \param digi_map Shared map associating collection names with digitization routines.
	 */
	explicit GRunAction(std::shared_ptr<GOptions>                           gopts,
						std::shared_ptr<gdynamicdigitization::dRoutinesMap> digi_map);

	~GRunAction() override = default;

	// The run action manages thread-local and process-wide state and is therefore
	// intentionally non-copyable and non-movable.
	GRunAction(const GRunAction&)            = delete;
	GRunAction& operator=(const GRunAction&) = delete;
	GRunAction(GRunAction&&)                 = delete;
	GRunAction& operator=(GRunAction&&)      = delete;

	/**
	 * \brief Returns the shared digitization-routine map used by this run action.
	 *
	 * Event processing uses this map to resolve the digitization routine associated
	 * with each hit collection name.
	 *
	 * \return Shared pointer to the digitization-routine map.
	 */
	[[nodiscard]] auto get_digitization_routines_map() const
		-> std::shared_ptr<gdynamicdigitization::dRoutinesMap> {
		return digitization_routines_map;
	}

	/**
	 * \brief Returns the worker-thread streamer map, if it has been instantiated.
	 *
	 * The map is created lazily on worker threads during
	 * the private method  \c BeginOfRunAction() when event-mode
	 * publication is required.
	 *
	 * \return Shared pointer to the worker-thread streamer map, or nullptr when unavailable.
	 */
	[[nodiscard]] auto get_streamer_threads_map() const
		-> std::shared_ptr<const gstreamer::gstreamersMap> {
		if (!gstreamer_threads_map) {
			log->error(ERR_STREAMERMAP_NOT_EXISTING, FUNCTION_NAME, " no gstreamer thread map available");
		}
		return gstreamer_threads_map;
	}

	/**
	 * \brief Adds one run-mode digitized payload to the current thread run-data collection.
	 *
	 * This method is used by event processing when a digitization routine reports
	 * that its output should be accumulated at run scope rather than published
	 * immediately at event scope.
	 *
	 * \param hcSDName Name of the hit collection or sensitive detector associated with the payload.
	 * \param digi_data Digitized payload to transfer into the run-level data collection.
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

	/**
	 * \brief Increments the number of events processed by the current thread for this run.
	 *
	 * The counter is stored in the run-data header and is updated once per processed event,
	 * independently of whether that event produces any payload.
	 */
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

	/**
	 * \brief Increments the number of events that produced run-mode payload.
	 *
	 * This counter is updated only for events that contributed at least one payload
	 * entry to the run-level accumulation path.
	 */
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
	 * \brief Moves the current worker-thread run data into the shared completed-run pool.
	 *
	 * Worker threads call this near run end so the master thread can later collect
	 * and merge the run-level results accumulated across all workers.
	 */
	void stash_worker_run_data();

	/**
	 * \brief Extracts all worker-completed run-data objects from the shared pool.
	 *
	 * This method is intended for master-thread use during
	 * \ref GRunAction::EndOfRunAction "EndOfRunAction()".
	 *
	 * \return Vector containing all completed worker run-data objects currently stored in the pool.
	 */
	[[nodiscard]] CompletedRunData take_completed_worker_run_data();

	/**
	 * \brief Creates the run object for the current execution thread.
	 *
	 * Geant4 calls this method after a run begins so the thread receives the run
	 * object associated with that execution context.
	 *
	 * \return Newly allocated GRun instance for the current thread.
	 */
	G4Run* GenerateRun() override;

	/**
	 * \brief Called by Geant4 at the beginning of a run.
	 *
	 * This method initializes the run-data bookkeeping for the current thread,
	 * inspects the digitization-routine map to determine required collection modes,
	 * and opens the appropriate streamer connections.
	 *
	 * Worker-thread behavior:
	 * - allocate run-data bookkeeping for the worker;
	 * - create the worker-thread streamer map on demand when event-mode publication is needed;
	 * - open all thread streamer connections.
	 *
	 * Master-thread behavior:
	 * - allocate run-data bookkeeping for the master;
	 * - create the run streamer map on demand when run-mode publication is needed;
	 * - open all master-side run streamer connections.
	 *
	 * \param run The Geant4 run descriptor corresponding to the run being started.
	 */
	void BeginOfRunAction(const G4Run* run) override;

	/**
	 * \brief Called by Geant4 at the end of a run.
	 *
	 * Worker-thread behavior:
	 * - close all worker-thread streamer connections;
	 * - move completed worker run data into the protected shared pool.
	 *
	 * Master-thread behavior:
	 * - collect and merge completed worker run data;
	 * - publish the merged run data through the master-side run streamers;
	 * - close all master-side run streamer connections.
	 *
	 * \param run The Geant4 run descriptor corresponding to the run being completed.
	 */
	void EndOfRunAction(const G4Run* run) override;

	/**
	 * \brief Publishes merged run data to all available master-side run streamers.
	 *
	 * \param run_data Shared run-data object containing the merged results to publish.
	 */
	void publish_run_data(const std::shared_ptr<GRunDataCollection>& run_data) const;

	/**
	 * \brief Shared configuration used by the run action and dependent run-scoped objects.
	 */
	std::shared_ptr<GOptions> goptions;

	/**
	 * \brief Shared digitization-routine map used to classify and process hit collections.
	 */
	std::shared_ptr<gdynamicdigitization::dRoutinesMap> digitization_routines_map;

	/**
	 * \brief Worker-thread streamer map, created lazily when event-mode publication is needed.
	 */
	std::shared_ptr<const gstreamer::gstreamersMap> gstreamer_threads_map;

	/**
	 * \brief Master-thread run streamer map, created lazily when run-mode publication is needed.
	 */
	std::shared_ptr<const gstreamer::gstreamersMap> gstreamer_run_map;

	/**
	 * \brief Per-thread run-level data collected during the current run.
	 *
	 * This object stores worker-side accumulation for run-mode digitizers and holds
	 * the counters maintained during event processing.
	 */
	std::unique_ptr<GRunDataCollection> run_data;

	/**
	 * \brief True when at least one digitization routine requires event-mode publication.
	 */
	bool need_a_thread_streamer = false;

	/**
	 * \brief True when at least one digitization routine requires run-mode accumulation/publication.
	 */
	bool need_a_run_streamer = false;

	/**
	 * \brief Mutex protecting access to the shared pool of completed worker run data.
	 */
	static std::mutex completed_run_data_mutex;

	/**
	 * \brief Shared storage containing worker-completed run data awaiting master collection.
	 *
	 * Worker threads append to this storage at run end, and the master thread extracts
	 * and clears it before performing merged run publication.
	 */
	static CompletedRunData completed_worker_run_data;
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