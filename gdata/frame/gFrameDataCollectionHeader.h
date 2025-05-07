#pragma once

/**
 * \file GFrameDataCollectionHeader.h
 * \brief Defines the header for frame data collection.
 *
 * The GFrameDataCollectionHeader contains the frame ID and frame duration,
 * and provides a timestamp computed from these values.
 */

#include "glogger.h"
#include <string>

class GFrameDataCollectionHeader {
public:
	/**
	 * \brief Constructs a GFrameDataCollectionHeader.
	 * \param frameID_ The frame ID.
	 * \param frameDuration_ The frame duration.
	 * \param logger Pointer to a GLogger instance.
	 */
	GFrameDataCollectionHeader(long int frameID_, float frameDuration_, std::shared_ptr<GLogger> logger)
		: log(std::move(logger)), frameID(frameID_), frameDuration(frameDuration_) {
		log->debug(CONSTRUCTOR, "GFrameHeader id ", frameID);
	}

	/**
	 * \brief Destructor for GFrameDataCollectionHeader.
	 */
	~GFrameDataCollectionHeader() { log->debug(DESTRUCTOR, "GFrameHeader id ", frameID); }

	/**
	 * \brief Gets the frame ID.
	 * \return The frame ID.
	 */
	[[nodiscard]] inline long int getFrameID() const { return frameID; }

	/**
	 * \brief Gets the computed time from frame ID and duration.
	 * \return The computed time.
	 */
	[[nodiscard]] inline long int getTime() const { return time_ns(); }

private:
	std::shared_ptr<GLogger> log;           ///< Logger instance
	long int                 frameID;       ///< Frame ID.
	float                    frameDuration; ///< Frame duration.

	/**
	 * \brief Computes a time value based on frame ID and duration.
	 * \return The computed time.
	 */
	[[nodiscard]] long int time_ns() const { return frameID * frameDuration; }
};

