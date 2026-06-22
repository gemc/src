#pragma once

// geant4
#include "G4UserEventAction.hh"

// gemc
#include <gemc/gbase/gbase.h>
#include <gemc/actions/run/gRunAction.h>

// c++
#include <chrono>

/**
 * \file gEventAction.h
 * \brief Declares GEventAction, the per-event processing action for the GEMC actions module.
 *
 * @ingroup gactions_module
 */

constexpr const char* EVENTACTION_LOGGER = "geventaction";

/**
 * \brief Name of the option controlling periodic per-event log messages.
 *
 * Accepts a string of the form \c N or \c N-NTH (N is the log module):
 * - \c N    : every worker thread prints a message each time it has processed a multiple of N
 *             events (the count is per thread, not the global event id).
 * - \c N-NTH: as above, but only the worker thread with id NTH prints (0 <= NTH < nthreads).
 */
constexpr const char* LOG_EVERY_OPTION = "log_every";

/**
 * \brief Namespace containing helpers related to event-action configuration.
 *
 * @ingroup gactions_module
 */
namespace geventaction {
	/**
	 * \brief Returns the options associated with the event-action logger scope.
	 *
	 * \return A GOptions object scoped to the event-action logger name.
	 */
	inline GOptions defineOptions() {
		GOptions goptions(EVENTACTION_LOGGER);

		std::string help = "Print a progress log line every N events, with the average event rate.\n \n";
		help += GTAB;
		help += "The value is a string of the form N or N-NTH, where N is the log module:\n";
		help += GTABTAB;
		help += "N     : every worker thread prints 'Starting event n. <k> in thread <tid>.\n";
		help += GTABTABTAB;
		help += "Average rate: <r> events / second' every time it has processed a multiple\n";
		help += GTABTABTAB;
		help += "of N events. <k> is that thread's own 1-based event count (not the global\n";
		help += GTABTABTAB;
		help += "Geant4 event id), and <r> is that thread's average rate.\n";
		help += GTABTAB;
		help += "N-NTH : as above, but only the worker thread with id NTH prints. NTH must be\n";
		help += GTABTABTAB;
		help += "in the range [0, nthreads-1].\n \n";
		help += GTAB;
		help += "Examples: -log_every=100 (all threads, every 100 events)\n";
		help += GTABTAB;
		help += "-log_every=100-2 (only thread 2, every 100 events)\n";
		goptions.defineOption(
			GVariable(LOG_EVERY_OPTION, UNINITIALIZEDSTRINGQUANTITY,
			          "log module: print event progress and average rate every N events per thread"),
			help);

		return goptions;
	}
} // namespace geventaction


/**
 * @class GEventAction
 * \brief Handles event begin/end callbacks, hit digitization, and event-level publication.
 *
 * This class is the worker-side event action used by GEMC. It participates in the
 * per-event lifecycle driven by Geant4 and is responsible for transforming hit
 * collections into GEMC data products.
 *
 * The event processing workflow implemented here is:
 * - begin-of-event logging and lightweight tracing;
 * - retrieval of the event hit collections at end of event;
 * - lookup of the digitization routine associated with each collection name;
 * - conversion of hits into digitized payload and true-information payload;
 * - storage of event-mode output in the event data collection;
 * - storage of run-mode output in the run-level collection owned by GRunAction;
 * - publication of completed event data through the worker-thread streamers.
 *
 * Ownership:
 * - The shared configuration object is retained by shared ownership.
 * - The run_action pointer is non-owning and is expected to remain valid for the
 *   lifetime of the worker-thread action set created by GAction.
 *
 * @ingroup gactions_module
 */
class GEventAction : public GBase<GEventAction>, public G4UserEventAction
{
public:
	/**
	 * \brief Constructs the event action.
	 *
	 * \param gopt Shared configuration object used for event-product construction and logging.
	 * \param run_a Non-owning pointer to the thread-local GRunAction instance that provides
	 *              access to digitization routines, run-level accumulation, and streamers.
	 */
	explicit GEventAction(const std::shared_ptr<GOptions>& gopt, GRunAction* run_a);

	~GEventAction() override = default;

	GEventAction(const GEventAction&)            = delete;
	GEventAction& operator=(const GEventAction&) = delete;
	GEventAction(GEventAction&&)                 = delete;
	GEventAction& operator=(GEventAction&&)      = delete;

	/**
	 * \brief Called by Geant4 at the beginning of an event.
	 *
	 * In this implementation, the method is mainly used for event/thread tracing and
	 * lightweight diagnostics.
	 *
	 * \param event Geant4 event descriptor for the event being started.
	 */
	void BeginOfEventAction(const G4Event* event) override;

	/**
	 * \brief Called by Geant4 at the end of an event.
	 *
	 * This method performs the main event-processing logic of the module:
	 * - count the event in the run-level bookkeeping;
	 * - retrieve the hit collections associated with the event;
	 * - resolve the proper digitization routine for each collection;
	 * - digitize each hit and collect its truth information;
	 * - route produced payload to event-mode or run-mode accumulation depending on
	 *   the digitizer collection mode;
	 * - publish the event data once all event-mode collections have been processed.
	 *
	 * \param event Geant4 event descriptor for the event being completed.
	 */
	void EndOfEventAction(const G4Event* event) override;

private:
	/**
	 * \brief Publishes a completed event data collection to all worker-thread streamers.
	 *
	 * \param event_data Shared event-data object containing the completed event payload.
	 */
	void publish_event_data(const std::shared_ptr<GEventDataCollection>& event_data) const;

	/**
	 * \brief Emits the periodic "Starting event" log line when enabled and due.
	 *
	 * Honors the \c log_every option: prints only when the log module is configured (> 0)
	 * and (when a thread filter is set) the running thread matches the requested one. The
	 * count is kept per worker thread, so the line is printed every time this thread has
	 * processed a multiple of N events. The reported event number is this thread's own
	 * 1-based count (not the global Geant4 event id), along with this thread's average rate
	 * (events / second) since its first counted event.
	 *
	 * \param thread_id Id of the worker thread processing the event.
	 */
	void log_event_start(int thread_id);

	/**
	 * \brief Number of events processed by this worker thread while logging is enabled.
	 */
	long long log_events_seen = 0;

	/**
	 * \brief Steady-clock anchor set on this thread's first counted event.
	 */
	std::chrono::steady_clock::time_point log_start_time;

	/**
	 * \brief Shared configuration used to build event products and control logging.
	 */
	std::shared_ptr<GOptions> goptions;

	/**
	 * \brief Event-id modulo for periodic logging; 0 disables the feature.
	 */
	int log_every_n = 0;

	/**
	 * \brief Worker-thread id allowed to log, or -1 to allow all threads.
	 */
	int log_every_thread = -1;

	/**
	 * \brief Non-owning pointer to the thread-local run action associated with this event action.
	 *
	 * The run action provides:
	 * - access to the shared digitization-routine map;
	 * - access to the worker-thread streamer map;
	 * - access to run-level accumulation counters and containers.
	 */
	GRunAction* run_action = nullptr;
};

// looping over output factories
// for (auto [factoryName, streamerFactory] : *gstreamerFactoryMap) {
// 	if (streamerFactory->getStreamType() == "event") {
// 		logSummary("Writing event data using streamer factory >" + factoryName + "<");
// 		map<string, bool> streamReport = streamerFactory->publishEventRunData(goptions, theRun->getRunData());
//
// 		for (auto [reportName, result] : streamReport) {
// 			string resultString = result ? " success" : " failure";
// 			logSummary("Factory <" + factoryName + "> " + reportName + resultString);
// 		}
// 	}
// }