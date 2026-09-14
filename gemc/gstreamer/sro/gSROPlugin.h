#pragma once

#include "gSROData.h"

/**
 * \file gSROPlugin.h
 * \brief Implementation contracts for SRO timing, crate processing, and frame output.
 *
 * Upcoming in the next release. GSROCrate provides the thread and queues; GSROService tracks event delivery.
 * GSROFactory loads GSROImplementation for runtime integration. These interfaces have no Geant4 or
 * output-format dependencies.
 */

/**
 * \brief Implementation-defined bound used by GEMC to establish safe input progress.
 *
 * Configure one timing model per run invocation. GEMC supplies the first event not fully delivered,
 * after checking the contiguous prefix of closed events and retention of their payloads in crate buffers.
 * Calls must be safe on a shared const instance, independent of worker scheduling.
 * GSROService calls this model on its progress thread, outside its mutex. Return promptly, do not call back
 * into the service, and throw on failure; the service cancels crate input and reports the error at shutdown.
 */
class GSROTiming
{
public:
	virtual ~GSROTiming() = default;

	/**
	 * \brief Lower bound for every possible payload from this event and all subsequent events.
	 *
	 * This is not merely the next event's start time. Account for negative electronics offsets and any
	 * non-monotonic source timestamps. Return std::nullopt when no finite bound can be guaranteed.
	 * A returned bound must not decrease as first_undelivered_event advances. All crates share this bound.
	 * GEMC must not use an event's completion count or its largest observed hit time as a substitute.
	 */
	[[nodiscard]] virtual std::optional<GSROTime>
	earliest_remaining_time(GSROEventId first_undelivered_event) const = 0;
};

/**
 * \brief Output destination called exclusively by its owning crate thread.
 *
 * One sink belongs to one crate. The implementation selects encoding and file structure. write_frame
 * takes ownership and must report failures by exception; success must not silently drop a frame.
 * Destruction releases resources, but checked flushing/closing belongs in finish_output().
 */
class GSROFrameSink
{
public:
	virtual ~GSROFrameSink() = default;
	virtual void write_frame(GSROFrame frame) = 0;
	virtual void finish_output() = 0;
};

/**
 * \brief Implementation-owned framing and optional electronics response for one crate.
 *
 * GEMC creates one instance per crate per run invocation. All callbacks execute serially on that crate's
 * thread. State may span frame boundaries; overlap processing must not reset at every frame boundary.
 * No mutable digitizer or worker state may be accessed here. The output sink outlives this instance.
 *
 * The collector accepts out-of-order deliveries, but calls consume_payload in ascending order of
 * (time, event_id, sequence), releasing only records strictly before a proven safe input boundary.
 * advance_time follows consumption of all records before that boundary. Equal-time records remain
 * pending until a later boundary, keeping their ordering deterministic.
 *
 * At orderly shutdown, including interruption, the collector consumes all remaining submitted records in
 * order before finish_run. Missing events can leave these records beyond the last safe input boundary.
 * Only advance_time and finish_run authorize frame emission; consume_payload alone does not establish
 * completeness. finish_run decides what to do with incomplete frames using the end context. GEMC imposes
 * no frame duration or acquisition cutoff. Output readiness may lag input progress when electronics
 * retain unresolved signals.
 *
 * On orderly shutdown, GEMC calls finish_run once, then output.finish_output once. No callbacks follow.
 * Callback exceptions propagate to the service, which stops dispatch and wakes blocked producers.
 * Successful finalization is not guaranteed after an exception; destructors still release resources.
 */
class GSROCratePlugin
{
public:
	explicit GSROCratePlugin(GSROFrameSink& sink) : output(sink) {}
	virtual ~GSROCratePlugin() = default;

	GSROCratePlugin(const GSROCratePlugin&) = delete;
	GSROCratePlugin& operator=(const GSROCratePlugin&) = delete;
	GSROCratePlugin(GSROCratePlugin&&) = delete;
	GSROCratePlugin& operator=(GSROCratePlugin&&) = delete;

	virtual void consume_payload(GSROPayload payload) = 0;
	virtual void advance_time(GSROTime safe_time) = 0;
	virtual void finish_run(const GSROEndContext& context) = 0;

protected:
	GSROFrameSink& output;
};
