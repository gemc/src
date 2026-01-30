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
 * The units are whatever the caller defines (ms, ns, ticks). The class does not
 * enforce units; it merely combines ID and duration consistently.
 */

#include "glogger.h"
#include <atomic>
#include <string>

class GFrameHeader {
public:
	/**
	 * \brief Construct a frame header.
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
	 */
	~GFrameHeader() { if (log) log->debug(DESTRUCTOR, "GFrameHeader id ", frameID); }

	/**
	 * \brief Test/example factory: create a header with a unique frame ID.
	 *
	 * The duration is set to a fixed example value. This is meant for examples/tests,
	 * not physics production.
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
	 * This is computed as \c frameID * frameDuration. Units are inherited from
	 * \c frameDuration and are therefore caller-defined.
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
	 * \note The method name keeps historical "ns" wording, but the computation
	 * does not enforce nanoseconds. Treat the units as caller-defined.
	 */
	[[nodiscard]] long int time_ns() const { return frameID * frameDuration; }

	/// Static thread-safe frame counter - used for testing/examples only.
	static std::atomic<int> globalFrameCounter;
};
