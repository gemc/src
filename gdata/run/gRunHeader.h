#pragma once

// gemc
#include "gbase.h"

// C++
#include <string>


/**
 * \file GRunHeader.h
 * \brief Defines the header for event data collection.
 *
 * This header contains event-related information such as the event number,
 * thread ID, and a timestamp. It is used by the GGlobarlDataCollection class.
 */

constexpr const char* GDATARUNHEADER_LOGGER = "run_header";

namespace grun_header {
inline auto defineOptions() -> GOptions {
    auto goptions = GOptions(GDATARUNHEADER_LOGGER);
    return goptions;
}
} // namespace grun_header


class GRunHeader : public GBase<GRunHeader>
{
public:
    /**
     * \brief Constructs a GGlobalHeader.
     *
     * \param gopts Pointer to a GOptions instance.
     * \param rid The Run ID.
     * \param tid The Thread ID.
     */
    GRunHeader(const std::shared_ptr<GOptions>& gopts, int rid, int tid = -1) :
        GBase(gopts, GDATARUNHEADER_LOGGER),
        runID(rid) {
        log->debug(CONSTRUCTOR, "GGlobalHeader");
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
     * \brief Gets RUN ID.
     * \return The RUN ID.
     */
    [[nodiscard]] auto getRunID() const -> int { return runID; }

    /**
     * \brief Gets the number of events for this run
     * \return The number of events for this run
     */
    [[nodiscard]] auto get_events_processed() const -> int { return events_processed; }


    void increment_events_processed() { events_processed++; }

private:
    int events_processed{0}; ///< number of events for this run
    int runID;              ///< RUN ID.
};
