#pragma once

/**
 * \file GEventDataCollection.h
 * \brief Defines the GEventDataCollection class, which aggregates event-level data.
 *
 * This class collects hit data for an event, keyed by the sensitive detector name.
 * It holds an event header and a map of GDataCollection pointers for each detector.
 */

#include "gEventHeader.h"
#include "gDataCollection.h"

// C++
#include <map>
#include <string>

constexpr const char* GEVENTDATA_LOGGER = "gevent_data";

namespace gevent_data {
inline auto defineOptions() -> GOptions {
    auto goptions = GOptions(GEVENTDATA_LOGGER);
    goptions      += geventheader::defineOptions();
    goptions      += gtrue_data::defineOptions();
    goptions      += gdigi_data::defineOptions();
    goptions      += gtouchable::defineOptions();
    return goptions;
}
} // namespace gevent_data


class GEventDataCollection : public GBase<GEventDataCollection>
{
public:
    /**
     * \brief Constructs a GEventDataCollection.
     *
     * \param gopts Pointer to GOptions
     * \param header Pointer to the event header.
     */

    GEventDataCollection(const std::shared_ptr<GOptions>& gopts, std::unique_ptr<GEventHeader> header)
        : GBase(gopts, GDATAEVENTHEADER_LOGGER), gevent_header(std::move(header)) {
    }

    /**
     * \brief Adds true hit information data for a detector.
     * \param sdName The sensitive detector name.
     * \param data Pointer to GTrueInfoData.
     */
    void addDetectorTrueInfoData(const std::string& sdName, std::unique_ptr<GTrueInfoData> data);

    /**
     * \brief Adds digitized hit data for a detector.
     * \param sdName The sensitive detector name.
     * \param data Pointer to GDigitizedData.
     */
    void addDetectorDigitizedData(const std::string& sdName, std::unique_ptr<GDigitizedData> data);

    /**
     * \brief Gets the event header.
     * \return Pointer to the event header.
     */
    [[nodiscard]] auto getHeader() const -> const std::unique_ptr<GEventHeader>& { return gevent_header; }

    /**
     * \brief Gets the map of data collections.
     * \return Pointer to the map from detector names to GDataCollection.
     */
    [[nodiscard]] auto getDataCollectionMap() const -> const std::map<std::string, std::unique_ptr<GDataCollection>>& {
        return gdataCollectionMap;
    }

    /**
     * \brief Gets the event number.
     * \return The event number.
     */
    [[nodiscard]] auto getEventNumber() const -> int { return gevent_header->getG4LocalEvn(); }

    // returning shared here as GEventDataCollection may be used by multiple streams and also collected in a runData vector
    static auto create(const std::shared_ptr<GOptions>& gopts) -> std::shared_ptr<GEventDataCollection> {
        auto header = GEventHeader::create(gopts);
        auto edc    = std::make_shared<GEventDataCollection>(gopts, std::move(header));

        auto digi_data = GDigitizedData::create(gopts);
        auto true_data = GTrueInfoData::create(gopts);

        edc->addDetectorDigitizedData("ctof", std::move(digi_data));
        edc->addDetectorTrueInfoData("ctof", std::move(true_data));

        return edc;
    }

private:
    std::unique_ptr<GEventHeader>                           gevent_header;
    std::map<std::string, std::unique_ptr<GDataCollection>> gdataCollectionMap; // keyed by the sensitive detector name.

    /// Static thread-safe event counter - used for testing only
    static std::atomic<int> globalEventDataCollectionCounter;
};
