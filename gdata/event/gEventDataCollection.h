#pragma once

/**
 * \file GEventDataCollection.h
 * \brief Defines the GEventDataCollection class, which aggregates event-level data.
 *
 * This class collects hit data for an event, keyed by sensitive detector name.
 * It holds an event header and a map of GDataCollection pointers for each detector.
 */

#include "gEventHeader.h"
#include "gDataCollection.h"

// C++
#include <map>
#include <string>

constexpr const char* GEVENTDATA_LOGGER = "gevent_data";

namespace gevent_data {
inline GOptions defineOptions() {
	auto goptions = GOptions(GEVENTDATA_LOGGER);
	goptions += gheader::defineOptions();
	goptions += gtrue_data::defineOptions();
	goptions += gdigi_data::defineOptions();
	goptions += gtouchable::defineOptions();
	return goptions;
}
}


class GEventDataCollection : public GBase<GEventDataCollection> {
public:
	/**
	 * \brief Constructs a GEventDataCollection.
	 *
	 * \param header Pointer to the event header.
	 * \param logger Pointer to a GLogger instance (using the 'gdata' name).
	 */

	GEventDataCollection(const std::shared_ptr<GOptions>& gopts, std::unique_ptr<GEventHeader> header)
		: GBase(gopts, GDATAEVENTHEADER_LOGGER), gheader(std::move(header)) { }

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
	[[nodiscard]] inline const std::unique_ptr<GEventHeader>& getHeader() const { return gheader; }

	/**
	 * \brief Gets the map of data collections.
	 * \return Pointer to the map from detector names to GDataCollection.
	 */
	[[nodiscard]] inline const std::map<std::string, std::unique_ptr<GDataCollection>>& getDataCollectionMap() const { return gdataCollectionMap; }

	/**
	 * \brief Gets the event number.
	 * \return The event number.
	 */
	[[nodiscard]] inline int getEventNumber() const { return gheader->getG4LocalEvn(); }


	// returning shared here as GEventDataCollection may be used by multiple streams and also collected in a runData vector
	static std::shared_ptr<GEventDataCollection> create(const std::shared_ptr<GOptions>& gopts) {
		auto header = GEventHeader::create(gopts);
		auto edc    = std::make_shared<GEventDataCollection>(gopts, std::move(header));

		auto digi_data = GDigitizedData::create(gopts);
		auto true_data = GTrueInfoData::create(gopts);

		edc->addDetectorDigitizedData("ctof", std::move(digi_data));
		edc->addDetectorTrueInfoData("ctof", std::move(true_data));

		return edc;
	}

private:
	std::unique_ptr<GEventHeader>                           gheader;
	std::map<std::string, std::unique_ptr<GDataCollection>> gdataCollectionMap;

	/// Static thread-safe event counter - used for testing only
	static std::atomic<int> globalEventDataCollectionCounter;

};
