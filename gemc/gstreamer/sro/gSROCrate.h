#pragma once

#include "gSROPlugin.h"

#include <functional>
#include <exception>
#include <memory>

/**
 * \brief Per-crate bounds preventing asynchronous delivery from consuming unlimited memory.
 *
 * Workers can produce payloads faster than a crate can process or write them. The input queue limits
 * make producers wait for space, allowing brief bursts without letting a slow consumer exhaust memory.
 * A single payload larger than queue_bytes is rejected immediately because waiting cannot make it fit.
 *
 * Limiting the queue alone is insufficient: the crate can drain it into the ordering buffer while an
 * earlier event is still running. Those pending payloads cannot reach the plugin until a safe-time
 * boundary proves their order. Separate pending limits bound this accumulation. Exceeding either fails
 * the crate and wakes producers; blocking the crate here could prevent it from receiving the earlier
 * payloads or progress messages needed to release pending data.
 *
 * Counts bound the number of records, including many small records; bytes account for variable-sized
 * contents such as waveforms. Byte limits count sizeof(GSROPayload) plus data->size_bytes() per payload.
 * Allocator/container overhead, producer-held input, and plugin-retained state are not included, so these
 * are not a total process-memory budget. Limits apply independently to each crate and must be positive.
 * They control framework storage, not the implementation's frame duration or incomplete-frame policy.
 */
struct GSROCrateLimits
{
	/// Maximum queued payload/progress messages; producers wait when the queue is full.
	std::size_t queue_messages = 1024;
	/// Maximum accounted payload bytes in the input queue; producers wait until their payload fits.
	std::size_t queue_bytes = 4 * 1024 * 1024;
	/// Maximum payloads awaiting time ordering; exceeding this fails the crate.
	std::size_t pending_payloads = 65536;
	/// Maximum accounted payload bytes awaiting time ordering; exceeding this fails the crate.
	std::size_t pending_bytes = 64 * 1024 * 1024;
};

/// Declare the sink first so the plugin referencing it is destroyed first.
struct GSROCrateResources
{
	std::unique_ptr<GSROFrameSink> sink;
	std::unique_ptr<GSROCratePlugin> plugin;
};

/**
 * \brief One crate's bounded input queue, ordering buffer, and background processing thread.
 *
 * Upcoming in the next release. Construction starts the thread; make_resources runs there and returns
 * the sink and the plugin bound to that sink. Their callbacks and destruction also run on that thread.
 * Keep implementation libraries loaded until this object is destroyed.
 *
 * enqueue_payload and advance_time support concurrent callers. The caller must establish global event
 * delivery before submitting a safe-time boundary; this class does not calculate one. Producers may
 * wait for queue space, but plugin callbacks and output never execute under the queue mutex.
 *
 * Payloads awaiting a safe boundary have separate limits. Exceeding them fails the crate instead of
 * blocking its consumer and preventing earlier input from arriving. These limits do not bound memory
 * retained inside the implementation plugin; a global admission policy remains a service-level concern.
 *
 * on_delivery acknowledges retention in the ordering buffer, not plugin processing or durable output.
 * It runs on the crate thread and must not block or call back into this crate. It may notify a future
 * event-delivery tracker. Its exceptions fail the crate just like plugin exceptions.
 * on_failure runs after a background failure, outside the queue mutex. It must not throw, block, or join
 * crate threads. A service may use it to cancel other crates; notification exceptions are suppressed.
 *
 * The owner must serialize finish_and_join calls and destruction, and keep this object alive until all
 * producers have returned. Callbacks must terminate; joining cannot interrupt a hung plugin or system call.
 */
class GSROCrate
{
public:
	using ResourceFactory = std::function<GSROCrateResources()>;
	using DeliveryCallback = std::function<void(GSROEventId, std::uint64_t)>;
	using FailureCallback = std::function<void(std::exception_ptr)>;

	GSROCrate(GSROCrateId crate_id, ResourceFactory make_resources,
	          GSROCrateLimits limits = {}, DeliveryCallback on_delivery = {}, FailureCallback on_failure = {});
	~GSROCrate();

	GSROCrate(const GSROCrate&) = delete;
	GSROCrate& operator=(const GSROCrate&) = delete;
	GSROCrate(GSROCrate&&) = delete;
	GSROCrate& operator=(GSROCrate&&) = delete;

	/// Transfers ownership. Rejects null data, wrong crates, oversized input, and timestamps before a boundary.
	void enqueue_payload(GSROPayload payload);

	/// Queue a nondecreasing boundary after all earlier input has been submitted. Equality is permitted.
	void advance_time(GSROTime safe_time);

	/// Close input and wake producers without joining; permits a service to stop every crate before waiting.
	void request_finish(GSROEndContext context);

	/// Fail input and wake waiters. Cleanup runs on the crate thread after any executing callback returns.
	void cancel(std::exception_ptr error);

	/**
	 * Stop accepting input, wake producers, drain, finalize, and join. Repeated calls only recheck errors.
	 * A supplied boundary must not precede any submitted boundary; absence preserves the latest one.
	 * Pending payloads reach the plugin before finish_run, leaving unfinished-frame policy to it.
	 * Rethrows background errors after joining. The destructor instead performs interrupted shutdown
	 * and suppresses errors, so explicit finalization is required to observe write/close failures.
	 */
	void finish_and_join(GSROEndContext context);

	/// Check for a background failure without waiting. Failure also wakes and rejects queued producers.
	void rethrow_if_failed() const;

private:
	class Impl;
	std::unique_ptr<Impl> impl;
};
