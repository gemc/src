#pragma once

/**
 * \file GFrameDataCollectionHeader.h
 * \brief Defines the header for frame data collection.
 *
 * The GFrameDataCollectionHeader contains the frame ID and frame duration,
 * and provides a timestamp computed from these values.
 */

#include "glogger.h"
#include <atomic>
#include <string>

class GFrameHeader {
public:
	/**
	 * \brief Constructs a GFrameDataCollectionHeader.
	 * \param frameID_ The frame ID.
	 * \param frameDuration_ The frame duration.
	 * \param logger Pointer to a GLogger instance.
	 */
	GFrameHeader(long int frameID_, double frameDuration_, std::shared_ptr<GLogger> logger)
		: frameID(frameID_), frameDuration(frameDuration_), log(logger)  { log->debug(CONSTRUCTOR, "GFrameHeader id ", frameID); }

	/**
	 * \brief Destructor for GFrameDataCollectionHeader.
	 */
	~GFrameHeader() { if (log) log->debug(DESTRUCTOR, "GFrameHeader id ", frameID); }


	static std::unique_ptr<GFrameHeader> create(std::shared_ptr<GLogger> logger) {
		double fd = 33.33; // Example frame duration (units could be ms or other)
		int eventNumber = globalFrameCounter.fetch_add(1, std::memory_order_relaxed);
		return std::make_unique<GFrameHeader>(eventNumber, fd, logger);
	}


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
	long int                 frameID;       ///< Frame ID.
	double                   frameDuration; ///< Frame duration.
	std::shared_ptr<GLogger> log{};         ///< Logger instance

	/**
	 * \brief Computes a time value based on frame ID and duration.
	 * \return The computed time.
	 */
	[[nodiscard]] long int time_ns() const { return frameID * frameDuration; }

	/// Static thread-safe event counter - used for testing only
	static std::atomic<int> globalFrameCounter;
};
