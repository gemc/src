#pragma once

/**
 * \file GRunDataCollection.h
 * \brief Defines the GRunDataCollection class, which aggregates run-level data.
 *
 * This class stores accumulated hit data for all events in a run, keyed by sensitive detector name.
 * It holds a run header and a map of GDataCollection pointers for each detector.
 */

#include "gRunHeader.h"
#include "gEventDataCollection.h"

// C++
#include <map>
#include <string>

constexpr const char* GRUNDATA_LOGGER = "grun_data";

namespace grun_data {
inline auto defineOptions() -> GOptions {
    auto goptions = GOptions(GRUNDATA_LOGGER);
    goptions      += geventheader::defineOptions();
    goptions      += gevent_data::defineOptions();
    goptions      += grun_header::defineOptions();
    goptions      += gtrue_data::defineOptions();
    goptions      += gdigi_data::defineOptions();
    goptions      += gtouchable::defineOptions();
    return goptions;
}
} // namespace grun_data


class GRunDataCollection : public GBase<GRunDataCollection>
{
public:
    /**
     * \brief Constructs a GRunDataCollection.
     *
	 * \param gopts Pointer to GOptions
     * \param header Pointer to the run header.
     */

    GRunDataCollection(const std::shared_ptr<GOptions>& gopts, std::unique_ptr<GRunHeader> header)
        : GBase(gopts, GRUNDATA_LOGGER), grun_header(std::move(header)) {
    }

    void collect_event_data_collection(const std::shared_ptr<GEventDataCollection> edc);


    /**
     * \brief Gets the event header.
     * \return Pointer to the event header.
     */
    [[nodiscard]] auto getHeader() const -> const std::unique_ptr<GRunHeader>& { return grun_header; }

    /**
     * \brief Gets the map of data collections.
     * \return Pointer to the map from detector names to GDataCollection.
     */
    [[nodiscard]] auto getDataCollectionMap() const -> const std::map<std::string, std::unique_ptr<GDataCollection>>& {
        return gdataCollectionMap;
    }

    /**
     * \brief Gets the run ID.
     * \return The run ID.
     */
    [[nodiscard]] auto getRunNumber() const -> int { return grun_header->getRunID(); }

    /**
     * \brief Gets the number of events processed.
     * \return The number of events processed.
     */
    [[nodiscard]] auto get_events_processed() const -> int { return grun_header->get_events_processed(); }

private:
    std::unique_ptr<GRunHeader>                             grun_header;
    std::map<std::string, std::unique_ptr<GDataCollection>> gdataCollectionMap; // keyed by the sensitive detector name.

    /**
     * \brief Collects true hit information data for a detector.
     * \param sdName The sensitive detector name.
     * \param data Pointer to GTrueInfoData.
     */
    void collectDetectorTrueInfoData(const std::string& sdName, const std::unique_ptr<GTrueInfoData>& data);

    /**
     * \brief Collects digitized hit data for a detector.
     * \param sdName The sensitive detector name.
     * \param data Pointer to GDigitizedData.
     */
    void collectDetectorDigitizedData(const std::string& sdName, const std::unique_ptr<GDigitizedData>& data);


};
