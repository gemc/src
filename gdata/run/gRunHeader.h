#pragma once

// gemc
#include "gbase.h"

// C++
#include <string>

/**
 * \file gRunHeader.h
 * \brief Defines GRunHeader, the metadata header associated with one run-level collection.
 *
 * \details
 * GRunHeader stores the minimal metadata needed by a run-level summary:
 * - the run identifier
 * - the number of processed events
 * - the number of processed events that contributed payload
 *
 * The object is typically owned by GRunDataCollection.
 */

constexpr const char* GDATARUNHEADER_LOGGER = "run_header";

namespace grun_header {

/**
 * \brief Defines the options subtree used by the run-header logger domain.
 *
 * \details
 * Higher-level modules can aggregate this option group into their own configuration bundles
 * so that run-header verbosity can be controlled centrally.
 *
 * \return Options group rooted at \c GDATARUNHEADER_LOGGER.
 */
inline auto defineOptions() -> GOptions {
	auto goptions = GOptions(GDATARUNHEADER_LOGGER);
	return goptions;
}

} // namespace grun_header

/**
 * \brief Stores minimal run metadata and run-level event counters.
 *
 * \details
 * Main responsibilities:
 * - preserve the run identifier
 * - track how many events were processed
 * - track how many processed events produced payload relevant to the run summary
 *
 * Ownership:
 * - this object is typically owned exclusively by GRunDataCollection
 *
 * Synchronization:
 * - this class does not perform internal synchronization
 * - concurrent updates require external coordination
 */
class GRunHeader : public GBase<GRunHeader>
{
public:
	/**
	 * \brief Constructs a run header.
	 *
	 * \details
	 * The constructor initializes the run identifier and leaves both counters at zero.
	 * It also emits a summary through the logger.
	 *
	 * \param gopts Shared options used to configure logging and related behavior.
	 * \param rid   Run identifier.
	 * \param tid   Optional thread identifier used for diagnostics, defaulting to \c -1.
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
	 * \brief Returns the run identifier.
	 *
	 * \return Run identifier.
	 */
	[[nodiscard]] auto getRunID() const -> int { return runID; }

	/**
	 * \brief Returns the total number of processed events tracked by this header.
	 *
	 * \return Number of processed events.
	 */
	[[nodiscard]] auto get_events_processed() const -> int { return events_processed; }

	/**
	 * \brief Returns the number of processed events that contributed payload.
	 *
	 * \return Number of payload-producing events.
	 */
	[[nodiscard]] auto get_events_with_payload() const -> int { return events_with_payload; }

	/**
	 * \brief Increments the processed-event counter by one.
	 *
	 * \details
	 * Intended to be called once per event entering the run summary flow.
	 */
	void increment_events_processed() { events_processed++; }

	/**
	 * \brief Increments the payload-producing-event counter by one.
	 *
	 * \details
	 * Intended to be called once for each processed event that actually contributes payload
	 * to the run summary.
	 */
	void increment_events_with_payload() { events_with_payload++; }

	/**
	 * \brief Adds an arbitrary number of processed events to the counter.
	 *
	 * \param count Number of processed events to add.
	 */
	void add_events_processed(int count) { events_processed += count; }

	/**
	 * \brief Adds an arbitrary number of payload-producing events to the counter.
	 *
	 * \param count Number of payload-producing events to add.
	 */
	void add_events_with_payload(int count) { events_with_payload += count; }

private:
	/// Total number of processed events represented by this run summary.
	int events_processed{0};

	/// Number of processed events that contributed payload to the run summary.
	int events_with_payload{0};

	/// Application-defined run identifier.
	int runID;
};