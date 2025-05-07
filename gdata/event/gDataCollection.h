#pragma once

/**
 * \file GDataCollection.h
 * \brief Defines the GDataCollection class for storing hit data for a detector.
 *
 * A GDataCollection holds collections of true hit data (GTrueInfoData) and
 * digitized hit data (GDigitizedData) for a single sensitive detector.
 */

#include "gTrueInfoData.h"
#include "gDigitizedData.h"
#include <vector>

class GDataCollection {
public:
	/**
	 * \brief Constructs a GDataCollection.
	 * \param logger Pointer to a GLogger instance.
	 */
	explicit GDataCollection(std::shared_ptr<GLogger> logger) : log(std::move(logger)) {
		log->debug(CONSTRUCTOR, "GDataCollection");
		trueInfosData = new std::vector<GTrueInfoData*>;
		digitizedData = new std::vector<GDigitizedData*>;
	}

	/**
	 * \brief Destructor for GDataCollection.
	 *
	 * Deletes all stored hit data and the associated containers.
	 */
	~GDataCollection() {
		for (auto* hit : (*trueInfosData)) { delete hit; }
		for (auto* hit : (*digitizedData)) { delete hit; }
		log->debug(DESTRUCTOR, "GDataCollection");
		delete trueInfosData;
		delete digitizedData;
	}

	/**
	 * \brief Adds true hit information data.
	 * \param data Pointer to GTrueInfoData.
	 */
	void addTrueInfoData(GTrueInfoData* data) const {
		log->debug(NORMAL, " adding hit to trueInfosData with identity: ", data->getIdentityString());
		trueInfosData->push_back(data);
	}

	/**
	 * \brief Adds digitized hit data.
	 * \param data Pointer to GDigitizedData.
	 */
	void addDigitizedData(GDigitizedData* data) const {
		log->debug(NORMAL, " adding hit to digitizedData with identity: ", data->getIdentityString());
		digitizedData->push_back(data);
	}

	/**
	 * \brief Returns the vector of true hit information data.
	 * \return Pointer to the vector of GTrueInfoData pointers.
	 */
	[[nodiscard]] inline const std::vector<GTrueInfoData*>* getTrueInfoData() const { return trueInfosData; }

	/**
	 * \brief Returns the vector of digitized hit data.
	 * \return Pointer to the vector of GDigitizedData pointers.
	 */
	[[nodiscard]] inline const std::vector<GDigitizedData*>* getDigitizedData() const { return digitizedData; }

private:
	std::vector<GTrueInfoData*>*  trueInfosData = nullptr; ///< Vector of true hit data.
	std::vector<GDigitizedData*>* digitizedData = nullptr; ///< Vector of digitized hit data.
	std::shared_ptr<GLogger>      log;                     ///< Logger instance
};

