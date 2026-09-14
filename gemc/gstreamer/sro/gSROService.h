#pragma once

#include "gSROCrate.h"

/**
 * \brief Shared, run-scoped registry routing worker payloads directly to crate threads.
 *
 * Upcoming in the next release. Own this service through a shared_ptr when sharing it across actions.
 * begin_run configures the implementation; no crate threads or files exist until a crate is requested.
 * With a timing model, a metadata-only progress thread broadcasts bounds; payloads bypass that thread.
 * Resource factories execute on their crate threads and must support concurrent calls for different crates.
 * Their captures must keep implementation libraries alive through shutdown.
 *
 * Creation and dispatch support concurrent workers. Registry locking never spans queue-space waits,
 * plugin calls, or thread joins. A background failure cancels every crate and rejects subsequent input.
 * Explicit finish_run joins all crates before reporting the first failure, even if multiple crates fail.
 *
 * The run owner serializes begin_run, advance_time, and finish_run. Before beginning another run or
 * destroying the service, it must wait for all producers from the previous run to return. No crate handles
 * escape the service. on_delivery runs on crate threads and must not call back into the service or block.
 *
 * In automatic mode, workers call complete_event after their last dispatch, including empty events.
 * An event advances the contiguous prefix only once closed and acknowledged by every destination crate.
 * Missing or unfinished events hold progress back; shutdown never silently marks them complete.
 * GSROFactory connects this service to GEMC actions. Finish policies belong to the implementation plugin.
 */
class GSROService
{
public:
	using ResourceFactory = std::function<GSROCrateResources(GSROCrateId)>;
	using DeliveryCallback = std::function<void(GSROCrateId, GSROEventId, std::uint64_t)>;

	GSROService();
	~GSROService();
	GSROService(const GSROService&) = delete;
	GSROService& operator=(const GSROService&) = delete;
	GSROService(GSROService&&) = delete;
	GSROService& operator=(GSROService&&) = delete;

	/// Begin a fresh invocation after the previous one was joined; clears its progress and failure state.
	void begin_run(ResourceFactory make_resources, GSROCrateLimits limits = {},
	               DeliveryCallback on_delivery = {});

	/**
	 * Begin with automatic progress using implementation-defined timing. The timing model must be non-null.
	 * max_pending_events bounds bookkeeping when an early event is slow or missing, including empty events.
	 * Exceeding it fails the run instead of blocking the worker that might need to complete the missing event.
	 * Event IDs start at zero for each invocation; UINT64_MAX is reserved to prevent prefix overflow.
	 */
	void begin_run(ResourceFactory make_resources, std::shared_ptr<const GSROTiming> timing,
	               GSROCrateLimits limits = {}, std::size_t max_pending_events = 65536);

	/// Also permits implementations to request output for empty crates. Concurrent requests create once.
	void create_crate_thread_if_needed(GSROCrateId crate_id);

	/// Transfers ownership directly to the destination queue, creating its crate on first use.
	void dispatch_payload_to_crate(GSROPayload payload);

	/// Automatic mode only. Close once, after all dispatch calls for this event have returned.
	void complete_event(GSROEventId event_id);

	/// Manual mode only: broadcast a proven bound after all earlier dispatches return. New crates inherit it.
	void advance_time(GSROTime safe_time);

	/// Close all input queues, drain and join all crates, then rethrow any failure. Repeated calls are safe.
	/// In automatic mode, stop/join workers first and leave context.safe_time empty: the service derives it
	/// after waiting for accepted deliveries. This finite bound is not the implementation's acquisition end.
	void finish_run(GSROEndContext context);
	/// Report a worker-side failure and wake blocked producers. The run owner still calls finish_run to join.
	void cancel_run(std::exception_ptr error);
	void rethrow_if_failed() const;

private:
	class Impl;
	std::unique_ptr<Impl> impl;
};
