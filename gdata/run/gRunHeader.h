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
 * - \c runID            : run identifier (application-defined)
 * - \c events_processed : number of events integrated into this run summary so far
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
 * \brief Defines \ref GOptions for the run-header logger domain.
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
 * \brief Minimal run metadata: run ID and integrated-event counter.
 *
 * \details
 * This object is typically owned by \ref GRunDataCollection as a \c std::unique_ptr.
 * It provides:
 * - stable access to run identifier
 * - a simple counter tracking how many events were integrated
 *
 * The counter is incremented via \ref GRunHeader::increment_events_processed "increment_events_processed()".
 *
 * \note
 * \ref GRunDataCollection does not automatically increment this counter in the current implementation.
 * If you want the value to reflect integrated events, ensure the caller (or the run collection)
 * invokes \ref GRunHeader::increment_events_processed "increment_events_processed()" once per event.
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
	 * - initial event count (usually 0)
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
			          TPOINTITEM, " Number of events collected:  ", events_processed,
			          TPOINTITEM, " Thread ID:  ", tid);
		}
		else {
			log->info(1, "\n",
			          TPOINTITEM, " Run ID:  ", rid, "\n",
			          TPOINTITEM, " Number of events collected:  ", events_processed);
		}
	}

	/**
	 * \brief Get the run identifier.
	 * \return Run ID.
	 */
	[[nodiscard]] auto getRunID() const -> int { return runID; }

	/**
	 * \brief Get the number of events integrated into this run summary so far.
	 *
	 * \details
	 * This value is incremented by \ref GRunHeader::increment_events_processed "increment_events_processed()".
	 * Typical usage is "once per event integrated into the run accumulator".
	 *
	 * \return Number of processed events.
	 */
	[[nodiscard]] auto get_events_processed() const -> int { return events_processed; }

	/**
	 * \brief Increment the number of processed events.
	 *
	 * \details
	 * Intended to be called once per event integrated into the run accumulator.
	 */
	void increment_events_processed() { events_processed++; }

private:
	int events_processed{0}; ///< Number of events integrated into the run summary.
	int runID;               ///< Run identifier.
};
