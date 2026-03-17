#pragma once

// gemc
#include "gbase.h"

// C++
#include <string>

/**
 * \file gRunHeader.h
 * \brief Defines \ref GRunHeader metadata for a run-level data collection.
 *
 * \details
 * A run header is a minimal metadata object associated with a \ref GRunDataCollection.
 *
 * It records:
 * - \c runID               : run identifier (application-defined)
 * - \c events_processed    : total number of events seen by the run action
 * - \c events_with_payload : number of events that contributed at least one run-mode payload
 *
 * The constructor emits a brief log summary. In multi-threaded contexts, an optional
 * thread ID can be attached for diagnostics and provenance.
 *
 * \note Synchronization
 * The header does not itself perform synchronization. If multiple threads are meant to update
 * \c events_processed concurrently, higher-level synchronization is required.
 */

constexpr const char* GDATARUNHEADER_LOGGER = "run_header";

namespace grun_header {
/**
 * \brief Defines GOptions for the run-header logger domain.
 *
 * \details
 * Higher-level option bundles (e.g. \ref grun_data::defineOptions "defineOptions()") typically include this.
 *
 * \return An options group rooted at the \ref GDATARUNHEADER_LOGGER domain.
 */
inline auto defineOptions() -> GOptions {
	auto goptions = GOptions(GDATARUNHEADER_LOGGER);
	return goptions;
}
} // namespace grun_header

/**
 * \brief Minimal run metadata: run ID and run-level event counters.
 *
 * \details
 * This object is typically owned by \ref GRunDataCollection as a \c std::unique_ptr.
 * It provides:
 * - stable access to run identifier
 * - a simple counter tracking how many events were processed
 * - a simple counter tracking how many events contributed run-mode payload
 *
 * The counters are incremented via:
 * - \ref GRunHeader::increment_events_processed "increment_events_processed()"
 * - \ref GRunHeader::increment_events_with_payload "increment_events_with_payload()"
 *
 * \note
 * \ref GRunDataCollection does not automatically decide which events contributed payload.
 * The caller (or higher-level run/event action code) must invoke:
 * - \ref GRunHeader::increment_events_processed "increment_events_processed()" once per processed event
 * - \ref GRunHeader::increment_events_with_payload "increment_events_with_payload()" once per event
 *   that produced at least one run-mode payload
 */
class GRunHeader : public GBase<GRunHeader>
{
public:
	/**
	 * \brief Construct a run header.
	 *
	 * \details
	 * The constructor logs:
	 * - run ID
	 * - initial total processed-event count (usually 0)
	 * - initial payload-event count (usually 0)
	 * - optional thread ID if provided
	 *
	 * \param gopts Shared options object used to configure logging and behavior.
	 * \param rid   Run identifier.
	 * \param tid   Optional thread ID for diagnostic labeling (default -1 = unspecified).
	 */
	GRunHeader(const std::shared_ptr<GOptions>& gopts, int rid, int tid = -1)
		: GBase(gopts, GDATARUNHEADER_LOGGER), runID(rid) {
		log->debug(CONSTRUCTOR, "GRunHeader");
		if (tid != -1) {
			log->info(1, "\n",
			          TPOINTITEM, " Run ID:  ", rid, "\n",
			          TPOINTITEM, " Number of events processed:  ", events_processed, "\n",
			          TPOINTITEM, " Number of events with payload:  ", events_with_payload, "\n",
			          TPOINTITEM, " Thread ID:  ", tid);
		}
		else {
			log->info(1, "\n",
			          TPOINTITEM, " Run ID:  ", rid, "\n",
			          TPOINTITEM, " Number of events processed:  ", events_processed, "\n",
			          TPOINTITEM, " Number of events with payload:  ", events_with_payload);
		}
	}

	/**
	 * \brief Get the run identifier.
	 * \return Run ID.
	 */
	[[nodiscard]] auto getRunID() const -> int { return runID; }

	/**
	 * \brief Get the total number of events processed in this run summary so far.
	 *
	 * \details
	 * This value is incremented by \ref GRunHeader::increment_events_processed "increment_events_processed()".
	 * Typical usage is "once per processed event reaching end-of-event handling".
	 *
	 * \return Number of processed events.
	 */
	[[nodiscard]] auto get_events_processed() const -> int { return events_processed; }

	/**
	 * \brief Get the number of events that contributed run-mode payload in this run summary so far.
	 *
	 * \details
	 * This value is incremented by
	 * \ref GRunHeader::increment_events_with_payload "increment_events_with_payload()".
	 * Typical usage is "once per event that contributed at least one run-mode payload".
	 *
	 * \return Number of payload-producing events.
	 */
	[[nodiscard]] auto get_events_with_payload() const -> int { return events_with_payload; }

	/**
	 * \brief Increment the number of processed events.
	 *
	 * \details
	 * Intended to be called once per processed event.
	 */
	void increment_events_processed() { events_processed++; }

	/**
	 * \brief Increment the number of events that produced run-mode payload.
	 *
	 * \details
	 * Intended to be called once per event that contributed at least one run-mode payload
	 * to the run accumulator.
	 */
	void increment_events_with_payload() { events_with_payload++; }

	/**
	 * \brief Add a number of processed events to the counter.
	 *
	 * \param count Number of processed events to add.
	 */
	void add_events_processed(int count) { events_processed += count; }

	/**
	 * \brief Add a number of payload-producing events to the counter.
	 *
	 * \param count Number of payload-producing events to add.
	 */
	void add_events_with_payload(int count) { events_with_payload += count; }

private:
	int events_processed{0};    ///< Total number of processed events in the run summary.
	int events_with_payload{0}; ///< Number of processed events that contributed run-mode payload.
	int runID;                  ///< Run identifier.
};