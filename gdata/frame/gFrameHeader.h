#pragma once

/**
 * \file gFrameHeader.h
 * \brief Defines the \ref GFrameHeader class, a minimal header describing a "frame".
 *
 * \details
 * A "frame" is a higher-level time grouping than an event. In streaming / readout-style
 * output, the system may produce integrated payloads over a fixed time window (frame).
 *
 * The \ref GFrameHeader stores:
 * - a frame identifier (\c frameID)
 * - a frame duration (\c frameDuration)
 *
 * From these it can compute a deterministic time coordinate (\ref GFrameHeader::getTime()).
 * The units are caller-defined (ms, ns, ticks, ...). The class does not enforce units; it
 * simply combines ID and duration consistently.
 *
 * \note The method name \ref GFrameHeader::time_ns() reflects historical naming; it does not enforce nanoseconds.
 */

#include "glogger.h"
#include <atomic>
#include <string>

class GFrameHeader {
public:
	/**
	 * \brief Construct a frame header.
	 *
	 * \details
	 * The constructor logs a debug message including the frame ID.
	 *
	 * \param frameID_       Frame index/identifier (monotonic in typical usage).
	 * \param frameDuration_ Duration of one frame in caller-defined units.
	 * \param logger         Logger used for construction/destruction diagnostics.
	 */
	GFrameHeader(long int frameID_, double frameDuration_, std::shared_ptr<GLogger> logger)
		: frameID(frameID_), frameDuration(frameDuration_), log(logger) {
		log->debug(CONSTRUCTOR, "GFrameHeader id ", frameID);
	}

	/**
	 * \brief Destructor (logs for debug builds/configurations).
	 *
	 * \details
	 * Logging is guarded by a null check to tolerate cases where the logger is absent.
	 */
	~GFrameHeader() { if (log) log->debug(DESTRUCTOR, "GFrameHeader id ", frameID); }

	/**
	 * \brief Test/example factory: create a header with a unique frame ID.
	 *
	 * \details
	 * The duration is set to a fixed example value. This is meant for examples/tests,
	 * not physics production.
	 *
	 * Threading:
	 * - Uses a global atomic counter so that concurrent calls can produce unique IDs.
	 *
	 * \param logger Logger instance.
	 * \return Newly created header.
	 */
	static std::unique_ptr<GFrameHeader> create(std::shared_ptr<GLogger> logger) {
		double fd      = 33.33; // Example frame duration (units could be ms or other)
		int    frameNo = globalFrameCounter.fetch_add(1, std::memory_order_relaxed);
		return std::make_unique<GFrameHeader>(frameNo, fd, logger);
	}

	/**
	 * \brief Get the frame identifier.
	 * \return Frame ID.
	 */
	[[nodiscard]] inline long int getFrameID() const { return frameID; }

	/**
	 * \brief Get a deterministic time coordinate for the frame.
	 *
	 * \details
	 * Computed as:
	 * \code
	 *   frameID * frameDuration
	 * \endcode
	 *
	 * Units are inherited from \c frameDuration and are therefore caller-defined.
	 *
	 * \return Time coordinate for this frame.
	 */
	[[nodiscard]] inline long int getTime() const { return time_ns(); }

private:
	long int frameID;        ///< Frame ID (index).
	double   frameDuration;  ///< Duration per frame (caller-defined units).
	std::shared_ptr<GLogger> log{}; ///< Logger instance.

	/**
	 * \brief Compute time coordinate based on ID and duration.
	 *
	 * \note Name is historical; the value is not necessarily in nanoseconds.
	 */
	[[nodiscard]] long int time_ns() const { return frameID * frameDuration; }

	/// Static thread-safe frame counter - used for testing/examples only.
	static std::atomic<int> globalFrameCounter;
};
