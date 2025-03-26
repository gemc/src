#ifndef GEVENTDATA_H
#define GEVENTDATA_H 1

/**
 * \file GEventDataCollection.h
 * \brief Defines the GEventDataCollection class, which aggregates event-level data.
 *
 * This class collects hit data for an event, keyed by sensitive detector name.
 * It holds an event header and a map of GDataCollection pointers for each detector.
 */

#include "gEventDataCollectionHeader.h"
#include "../gdataConventions.h"
#include "gDataCollection.h"
#include <vector>
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
	GEventDataCollection(GEventDataCollectionHeader *header, GLogger * const logger) : log(logger), gheader(header) {
		log->debug(CONSTRUCTOR, "GEventDataCollection");
		gdataCollectionMap = new std::map<std::string, GDataCollection*>();
	}

	/**
	 * \brief Destructor for GEventDataCollection.
	 *
	 * Deletes the event header, the data collection map (and its contents), and the logger.
	 */
	~GEventDataCollection() {
		log->debug(DESTRUCTOR, "GEventDataCollection");
		for (auto &[keys, values] : (*gdataCollectionMap)) {
			delete values;
		}
		delete gheader;
		delete gdataCollectionMap;
	}

	/**
	 * \brief Adds true hit information data for a detector.
	 * \param sdName The sensitive detector name.
	 * \param data Pointer to GTrueInfoData.
	 */
	void addDetectorTrueInfoData(std::string sdName, GTrueInfoData *data);

	/**
	 * \brief Adds digitized hit data for a detector.
	 * \param sdName The sensitive detector name.
	 * \param data Pointer to GDigitizedData.
	 */
	void addDetectorDigitizedData(std::string sdName, GDigitizedData *data);

	/**
	 * \brief Gets the event header.
	 * \return Pointer to the event header.
	 */
	inline const GEventDataCollectionHeader *getHeader() const { return gheader; }

	/**
	 * \brief Gets the map of data collections.
	 * \return Pointer to the map from detector names to GDataCollection.
	 */
	inline const std::map<std::string, GDataCollection*> *getDataCollectionMap() const { return gdataCollectionMap; }

	/**
	 * \brief Gets the event number.
	 * \return The event number.
	 */
	inline int getEventNumber() const { return gheader->getG4LocalEvn(); }

	/**
	 * \brief Gets the true hit information data for a specific detector.
	 * \param detector The detector name.
	 * \return Pointer to a vector of GTrueInfoData pointers, or nullptr if not found.
	 */
	const std::vector<GTrueInfoData*> *getTrueInfoDataForDetector(std::string detector) const;

	/**
	 * \brief Gets the digitized hit data for a specific detector.
	 * \param detector The detector name.
	 * \return Pointer to a vector of GDigitizedData pointers, or nullptr if not found.
	 */
	const std::vector<GDigitizedData*> *getDigitizedDataForDetector(std::string detector) const;

private:
	GLogger * const log;                         ///< Logger instance.
	GEventDataCollectionHeader *gheader = nullptr; ///< Event header.
	std::map<std::string, GDataCollection*> *gdataCollectionMap; ///< Map of data collections keyed by detector name.
};

#endif
