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
 * A run header records:
 * - the run identifier (\c runID)
 * - how many events have been integrated into this run summary (\c events_processed)
 *
 * The constructor logs a short summary. In a multi-threaded environment, the thread ID
 * can be attached for diagnostics.
 */

constexpr const char* GDATARUNHEADER_LOGGER = "run_header";

namespace grun_header {
/**
 * \brief Defines GOptions for the run-header logger domain.
 */
inline auto defineOptions() -> GOptions {
	auto goptions = GOptions(GDATARUNHEADER_LOGGER);
	return goptions;
}
} // namespace grun_header

class GRunHeader : public GBase<GRunHeader>
{
public:
	/**
	 * \brief Construct a run header.
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
	 * This value is incremented by \ref increment_events_processed().
	 *
	 * \return Number of processed events.
	 */
	[[nodiscard]] auto get_events_processed() const -> int { return events_processed; }

	/**
	 * \brief Increment the number of processed events.
	 *
	 * Intended to be called once per event that is integrated into the run accumulator.
	 */
	void increment_events_processed() { events_processed++; }

private:
	int events_processed{0}; ///< Number of events integrated into the run summary.
	int runID;               ///< Run identifier.
};
