#ifndef GFRAMEHEADER_H
#define GFRAMEHEADER_H 1

/**
 * \file GFrameDataCollectionHeader.h
 * \brief Defines the header for frame data collection.
 *
 * The GFrameDataCollectionHeader contains the frame ID and frame duration,
 * and provides a timestamp computed from these values.
 */

#include "../gdataConventions.h"
#include "glogger.h"
#include <iostream>
#include <string>

class GFrameDataCollectionHeader {
public:
	/**
	 * \brief Constructs a GFrameDataCollectionHeader.
	 * \param frameID_ The frame ID.
	 * \param frameDuration_ The frame duration.
	 * \param logger Pointer to a GLogger instance.
	 */
	GFrameDataCollectionHeader(long int frameID_, float frameDuration_, GLogger *const logger)
			:  log(logger), frameID(frameID_), frameDuration(frameDuration_) {
		log->debug(CONSTRUCTOR, "GFrameHeader id ", frameID);
	}

	/**
	 * \brief Destructor for GFrameDataCollectionHeader.
	 */
	~GFrameDataCollectionHeader() {
		log->debug(DESTRUCTOR, "GFrameHeader id ", frameID);
	}

	/**
	 * \brief Gets the frame ID.
	 * \return The frame ID.
	 */
	inline long int getFrameID() const { return frameID; }

	/**
	 * \brief Gets the computed time from frame ID and duration.
	 * \return The computed time.
	 */
	inline long int getTime() const { return time_ns(); }

private:
	GLogger *const log;   ///< Logger instance.
	long int frameID;     ///< Frame ID.
	float frameDuration;  ///< Frame duration.

	/**
	 * \brief Computes a time value based on frame ID and duration.
	 * \return The computed time.
	 */
	long int time_ns() const { return frameID * frameDuration; }
};

#endif
