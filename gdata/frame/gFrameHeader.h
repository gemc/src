#pragma once

/**
 * \file gFrameHeader.h
 * \brief Defines the GFrameHeader class, a minimal header describing a frame.
 *
 * \details
 * A frame is a higher-level time grouping than an event. In streaming/readout-style output,
 * the system may produce integrated payloads over a fixed time window.
 *
 * The GFrameHeader stores:
 * - a frame identifier (\c frameID)
 * - a frame duration (\c frameDuration)
 *
 * From these values it can compute a deterministic time coordinate via
 * \ref GFrameHeader::getTime "getTime()".
 *
 * Units:
 * - units are caller-defined, for example ms, ns, or ticks
 * - the class does not enforce units
 * - it simply combines ID and duration consistently
 *
 * Threading:
 * - the static factory \ref GFrameHeader::create "create()" uses a thread-safe global counter for examples/tests
 */

#include "glogger.h"
#include <atomic>
#include <string>

/**
 * \defgroup gdata_frame_header GData frame header
 * \brief Minimal metadata describing one frame-style time window.
 *
 * \details
 * This topic documents the header object used by frame-based output. It keeps a frame identifier
 * and duration and provides a deterministic derived time coordinate.
 */

/**
 * \brief Stores minimal metadata for one frame-style time window.
 * \ingroup gdata_frame_header
 */
class GFrameHeader
{
public:
	/**
	 * \brief Construct a frame header.
	 *
	 * \details
	 * The constructor stores the provided identifier and duration and emits a debug message.
	 *
	 * Ownership:
	 * - the logger is stored as a shared pointer and may be shared across many headers/collections
	 *
	 * \param frameID_       Frame index or identifier, typically monotonic in usage.
	 * \param frameDuration_ Duration of one frame in caller-defined units.
	 * \param logger         Logger used for construction/destruction diagnostics.
	 */
	GFrameHeader(long int frameID_, double frameDuration_, std::shared_ptr<GLogger> logger)
		: frameID(frameID_), frameDuration(frameDuration_), log(logger) {
		log->debug(CONSTRUCTOR, "GFrameHeader id ", frameID);
	}

	/**
	 * \brief Destructor.
	 *
	 * \details
	 * A debug message is emitted when a logger is available.
	 * Logging is guarded by a null check so destruction remains tolerant of absent logger instances.
	 */
	~GFrameHeader() { if (log) log->debug(DESTRUCTOR, "GFrameHeader id ", frameID); }

	/**
	 * \brief Test/example factory creating a header with a unique frame ID.
	 *
	 * \details
	 * The duration is set to a fixed example value.
	 * This helper is intended for examples and tests rather than production payload generation.
	 *
	 * Threading:
	 * - uses a global atomic counter so concurrent calls can produce unique IDs
	 *
	 * \param logger Logger instance.
	 * \return Newly created header.
	 */
	static std::unique_ptr<GFrameHeader> create(std::shared_ptr<GLogger> logger) {
		double fd      = 33.33; // Example frame duration (units could be ms or other).
		int    frameNo = globalFrameCounter.fetch_add(1, std::memory_order_relaxed);
		return std::make_unique<GFrameHeader>(frameNo, fd, logger);
	}

	/**
	 * \brief Returns the frame identifier.
	 *
	 * \return Frame ID.
	 */
	[[nodiscard]] inline long int getFrameID() const { return frameID; }

	/**
	 * \brief Returns a deterministic time coordinate for the frame.
	 *
	 * \details
	 * Computed as:
	 * \code
	 * frameID * frameDuration
	 * \endcode
	 *
	 * Units are inherited from \c frameDuration and are therefore caller-defined.
	 *
	 * \return Time coordinate for this frame.
	 */
	[[nodiscard]] inline long int getTime() const { return time_ns(); }

private:
	long int                 frameID;       ///< Frame ID or frame index.
	double                   frameDuration; ///< Duration of one frame in caller-defined units.
	std::shared_ptr<GLogger> log{};         ///< Logger instance shared with related frame objects.

	/**
	 * \brief Computes the frame time coordinate from ID and duration.
	 *
	 * \details
	 * The function name is historical. The returned value is not guaranteed to be in nanoseconds.
	 *
	 * \return \c frameID * frameDuration cast to \c long int.
	 */
	[[nodiscard]] long int time_ns() const { return frameID * frameDuration; }

	/// Static thread-safe frame counter used for testing/examples only.
	static std::atomic<int> globalFrameCounter;
};