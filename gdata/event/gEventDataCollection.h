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
#include <map>
#include <string>

class GEventDataCollection {
public:
	/**
	 * \brief Constructs a GEventDataCollection.
	 *
	 * \param header Pointer to the event header.
	 * \param logger Pointer to a GLogger instance (using the 'gdata' name).
	 */

	GEventDataCollection(std::unique_ptr<GEventHeader> header, std::shared_ptr<GLogger> logger)
		: log(logger), gheader(std::move(header)) { log->debug(CONSTRUCTOR, "GEventDataCollection"); }

	/**
	 * \brief Destructor for GEventDataCollection.
	 *
	 * Deletes the event header, the data collection map (and its contents), and the logger.
	 */
	~GEventDataCollection() { if (log) log->debug(DESTRUCTOR, "GEventDataCollection"); }

	/**
	 * \brief Adds true hit information data for a detector.
	 * \param sdName The sensitive detector name.
	 * \param data Pointer to GTrueInfoData.
	 */
	void addDetectorTrueInfoData(std::string sdName, std::unique_ptr<GTrueInfoData> data);

	/**
	 * \brief Adds digitized hit data for a detector.
	 * \param sdName The sensitive detector name.
	 * \param data Pointer to GDigitizedData.
	 */
	void addDetectorDigitizedData(std::string sdName, std::unique_ptr<GDigitizedData> data);

	/**
	 * \brief Gets the event header.
	 * \return Pointer to the event header.
	 */
	[[nodiscard]] inline const std::unique_ptr<GEventHeader>& getHeader() const { return gheader; }

	/**
	 * \brief Gets the map of data collections.
	 * \return Pointer to the map from detector names to GDataCollection.
	 */
	[[nodiscard]] inline const std::map<std::string, std::unique_ptr<GDataCollection>>& getDataCollectionMap() const {
		return gdataCollectionMap;
	}

	/**
	 * \brief Gets the event number.
	 * \return The event number.
	 */
	[[nodiscard]] inline int getEventNumber() const { return gheader->getG4LocalEvn(); }


	// returning shared here as GEventDataCollection may be used by multiple streams and also collected in a runData vector
	static std::shared_ptr<GEventDataCollection> create(std::shared_ptr<GLogger> logger) {

		auto header = GEventHeader::create(logger);

		auto digi_data = GDigitizedData::create(logger);
		auto true_data = GTrueInfoData::create(logger);

		auto edc = std::make_shared<GEventDataCollection>(std::move(header), logger);
		edc->addDetectorDigitizedData("ctof", std::move(digi_data));
		edc->addDetectorTrueInfoData("ctof", std::move(true_data));

		return edc;
	}

private:
	std::shared_ptr<GLogger>                                log; ///< Logger instance
	std::unique_ptr<GEventHeader>                           gheader;
	std::map<std::string, std::unique_ptr<GDataCollection>> gdataCollectionMap;

	/// Static thread-safe event counter - used for testing only
	static std::atomic<int> globalEventDataCollectionCounter;

};
