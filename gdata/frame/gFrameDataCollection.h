#ifndef GFRAMEDATA_H
#define GFRAMEDATA_H 1

/**
 * \file GFrameDataCollection.h
 * \brief Defines the GFrameDataCollection class for frame-level data.
 *
 * This class collects integral payloads for a frame and is associated with a frame header.
 */

#include "gFrameDataCollectionHeader.h"
#include "gIntegralPayload.h"
#include "../gdataConventions.h"
#include <vector>
#include <string>

class GFrameDataCollection {
public:
	/**
	 * \brief Constructs a GFrameDataCollection.
	 * \param header Pointer to a GFrameDataCollectionHeader.
	 * \param logger Pointer to a GLogger instance.
	 */
	GFrameDataCollection(GFrameDataCollectionHeader *header, GLogger *const logger)
			: log(logger), gheader(header)  {
		log->debug(CONSTRUCTOR, "GFrameDataCollection");
		integralPayloads = new std::vector<GIntegralPayload*>();
	}

	/**
	 * \brief Destructor for GFrameDataCollection.
	 */
	~GFrameDataCollection() {
		log->debug(DESTRUCTOR, "GFrameDataCollection");
		delete gheader;
		for (auto *payload : *integralPayloads) {
			delete payload;
		}
		delete integralPayloads;
	}

public:
	/**
	 * \brief Adds an integral payload.
	 *
	 * Expects the payload vector to have exactly 5 elements:
	 * crate, slot, channel, charge, and time.
	 *
	 * \param payload A vector of integers representing the payload.
	 */
	void addIntegralPayload(std::vector<int> payload) {
		if (payload.size() == 5) {
			int crate   = payload[0];
			int slot    = payload[1];
			int channel = payload[2];
			int charge  = payload[3];
			int time    = payload[4];

			// Use 'log' as the logger for GIntegralPayload.
			GIntegralPayload *gpayload = new GIntegralPayload(crate, slot, channel, charge, time, log);
			integralPayloads->push_back(gpayload);
			log->debug(NORMAL, " adding integral payload for crate ", crate, " slot ", slot, " channel ", channel, " charge ", charge, " time ", time);
		} else {
			log->error(EC__WRONGPAYLOAD, "payload size is not 5 but ", payload.size());
		}
	}

	// Placeholder: Add event-specific data.
	void addEvent(int evn);

	// Placeholder: Determine if the frame should be written.
	bool shouldWriteFrame() const;

	/**
	 * \brief Gets the frame header.
	 * \return Pointer to the frame header.
	 */
	inline const GFrameDataCollectionHeader *getHeader() const { return gheader; }

	/**
	 * \brief Gets the integral payloads.
	 * \return Pointer to a vector of GIntegralPayload pointers.
	 */
	inline const std::vector<GIntegralPayload*> *getIntegralPayload() const { return integralPayloads; }

	/**
	 * \brief Gets the frame ID.
	 * \return The frame ID.
	 */
	inline long int getFrameID() const { return gheader->getFrameID(); }

private:
	GLogger *const log;                             ///< Logger instance.
	GFrameDataCollectionHeader *gheader = nullptr;    ///< Frame header.
	std::vector<GIntegralPayload*> *integralPayloads; ///< Vector of integral payloads.
};

#endif
