#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>

/**
 * \file gSROData.h
 * \brief Transport types for the SRO interfaces, upcoming in the next release.
 *
 * Payload production belongs to worker digitizers;
 * framing, electronics response, and encoding belong to the experiment's implementation.
 */

using GSROCrateId = std::uint32_t;
using GSROEventId = std::uint64_t;
using GSROTime = std::chrono::duration<std::int64_t, std::nano>;

/**
 * \brief Base for implementation-defined payload and frame contents.
 *
 * Derive concrete types in the implementation repository, such as clas12-systems. Objects must own their
 * contents and must not borrow Geant4 hits, event collections, or mutable worker state. Transfer ownership
 * through the envelope; the producer must not retain mutable aliases. Keep the defining plugin library
 * loaded until all its data objects have been destroyed.
 */
class GSROData
{
public:
	virtual ~GSROData() = default;

	/// Resident bytes, including this concrete object and its owned allocations, for buffer accounting.
	[[nodiscard]] virtual std::size_t size_bytes() const noexcept = 0;
};

/// Context passed to worker digitizers. Event IDs restart at zero for each run invocation.
struct GSROEventContext {
	int run_id;
	GSROEventId event_id;
};

/// Immediate worker dispatch; never retain this callback beyond stream_hit. The action assigns sequence IDs.
using GSROEmit = std::function<void(GSROCrateId, GSROTime, std::unique_ptr<const GSROData>)>;

/**
 * \brief One worker-produced contribution, transferred by move to its destination crate.
 *
 * All timestamps use a common simulation origin for the current run; they are not wall-clock readings.
 * The implementation converts its native time units to GSROTime. Negative times are permitted.
 * Event IDs are zero-based within one run invocation, independent of worker IDs. sequence is unique within
 * that event and assigned by the worker, providing a stable ordering key for equal timestamps.
 * data must be non-null and of a type understood by the destination implementation.
 */
struct GSROPayload
{
	GSROCrateId crate_id;
	GSROEventId event_id;
	std::uint64_t sequence;
	GSROTime time;
	std::unique_ptr<const GSROData> data;
};

/**
 * \brief An implementation-produced frame, transferred by move to the output sink.
 *
 * The implementation chooses frame IDs and intervals [begin, end), with begin < end. data must be non-null.
 * Contents may be typed electronics records or already encoded bytes. Completeness, partial-frame flags,
 * and encoding are implementation policy; the framework does not infer them from the interval.
 */
struct GSROFrame
{
	GSROCrateId crate_id;
	std::uint64_t frame_id;
	GSROTime begin;
	GSROTime end;
	std::unique_ptr<const GSROData> data;
};

enum class GSROEndReason { completed, interrupted };

/**
 * \brief Input-delivery state passed to the implementation at orderly shutdown.
 *
 * All submitted payloads have been drained before this notification. On interruption, events may be
 * missing even though later events delivered payloads. safe_time is the last proven input boundary:
 * no further input strictly earlier than it is outstanding. Absence means no finite boundary was proven.
 * This is not an acquisition end time. The implementation defines acquisition limits and decides whether
 * to emit, mark, or discard unfinished frames. Completion does not implicitly advance time to infinity.
 */
struct GSROEndContext
{
	GSROEndReason reason;
	std::optional<GSROTime> safe_time;
};
