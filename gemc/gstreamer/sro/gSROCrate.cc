#include "gSROCrate.h"

#include <gemc/guts/gthreads.h>

#include <condition_variable>
#include <deque>
#include <exception>
#include <map>
#include <mutex>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <variant>

class GSROCrate::Impl
{
public:
	Impl(GSROCrateId id, ResourceFactory factory, GSROCrateLimits bounds,
	     DeliveryCallback delivered, FailureCallback failed)
		: crate_id(id), make_resources(std::move(factory)), limits(bounds),
		  on_delivery(std::move(delivered)), on_failure(std::move(failed)) {
		if (!make_resources || !limits.queue_messages || !limits.queue_bytes ||
		    !limits.pending_payloads || !limits.pending_bytes) {
			throw std::invalid_argument("SRO crate requires a resource factory and positive buffer limits");
		}
		thread = jthread_alias([this] { run(); });
	}

	void enqueue_payload(GSROPayload payload) {
		if (!payload.data || payload.crate_id != crate_id) {
			throw std::invalid_argument("SRO payload is null or addressed to another crate");
		}
		const auto bytes = payload.data->size_bytes();
		if (limits.queue_bytes < sizeof(GSROPayload) || bytes > limits.queue_bytes - sizeof(GSROPayload)) {
			throw std::length_error("SRO payload exceeds the crate input queue byte limit");
		}
		const auto total_bytes = bytes + sizeof(GSROPayload);
		std::unique_lock lock(mutex);
		space_available.wait(lock, [&] {
			return closing || failure ||
			       (queue.size() < limits.queue_messages && total_bytes <= limits.queue_bytes - queue_bytes);
		});
		check_open();
		if (submitted_safe_time && payload.time < *submitted_safe_time) {
			throw std::invalid_argument("SRO payload precedes an already submitted safe-time boundary");
		}
		queue.emplace_back(QueuedPayload{std::move(payload), total_bytes});
		queue_bytes += total_bytes;
		lock.unlock();
		input_available.notify_one();
	}

	void advance_time(GSROTime safe_time) {
		std::unique_lock lock(mutex);
		space_available.wait(lock, [&] { return closing || failure || queue.size() < limits.queue_messages; });
		check_open();
		if (submitted_safe_time && safe_time < *submitted_safe_time) {
			throw std::invalid_argument("SRO safe-time boundaries must not decrease");
		}
		queue.emplace_back(safe_time);
		submitted_safe_time = safe_time;
		lock.unlock();
		input_available.notify_one();
	}

	void request_finish(GSROEndContext context) {
		{
			std::lock_guard lock(mutex);
			if (!closing && !failure) {
				if (context.safe_time && submitted_safe_time && *context.safe_time < *submitted_safe_time) {
					throw std::invalid_argument("SRO final boundary precedes an already submitted boundary");
				}
				if (!context.safe_time) { context.safe_time = submitted_safe_time; }
				end_context = context;
				closing = true;
			}
		}
		input_available.notify_one();
		space_available.notify_all();
	}

	void cancel(std::exception_ptr error) {
		if (!error) { throw std::invalid_argument("SRO cancellation requires an exception"); }
		{
			std::lock_guard lock(mutex);
			if (!failure) { failure = error; }
			closing = true;
		}
		input_available.notify_one();
		space_available.notify_all();
	}

	void finish_and_join(GSROEndContext context) {
		request_finish(context);
		if (thread.joinable()) { thread.join(); }
		rethrow_if_failed();
	}

	void rethrow_if_failed() const {
		std::lock_guard lock(mutex);
		if (failure) { std::rethrow_exception(failure); }
	}

private:
	struct QueuedPayload {
		GSROPayload payload;
		std::size_t bytes;
	};
	using Message = std::variant<QueuedPayload, GSROTime>;
	using Key = std::tuple<GSROTime, GSROEventId, std::uint64_t>;
	using Pending = std::map<Key, QueuedPayload>;

	void check_open() const {
		if (failure) { std::rethrow_exception(failure); }
		if (closing) { throw std::logic_error("SRO crate is closed to input"); }
	}

	void consume_before(Pending& pending, std::size_t& bytes, GSROCratePlugin& plugin,
	                    std::optional<GSROTime> boundary) {
		while (!pending.empty() && (!boundary || std::get<0>(pending.begin()->first) < *boundary)) {
			rethrow_if_failed();
			auto entry = pending.extract(pending.begin());
			bytes -= entry.mapped().bytes;
			plugin.consume_payload(std::move(entry.mapped().payload));
		}
	}

	void run() noexcept {
		try {
			// Plugin and sink are local so destruction happens here, in dependency order, even on failure.
			auto resources = make_resources();
			if (!resources.sink || !resources.plugin) {
				throw std::invalid_argument("SRO resource factory returned a null plugin or sink");
			}
			Pending pending;
			std::size_t pending_bytes = 0;
			std::optional<GSROTime> processed_safe_time;
			GSROEndContext context{GSROEndReason::interrupted, std::nullopt};
			for (;;) {
				Message message;
				{
					std::unique_lock lock(mutex);
					input_available.wait(lock, [&] { return closing || !queue.empty(); });
					if (failure) { std::rethrow_exception(failure); }
					if (queue.empty()) {
						context = *end_context;
						break;
					}
					message = std::move(queue.front());
					queue.pop_front();
					if (const auto* item = std::get_if<QueuedPayload>(&message)) { queue_bytes -= item->bytes; }
				}
				space_available.notify_all();
				if (auto* item = std::get_if<QueuedPayload>(&message)) {
					if (pending.size() >= limits.pending_payloads ||
					    item->bytes > limits.pending_bytes - pending_bytes) {
						throw std::length_error("SRO pending input limit exceeded while waiting for safe time");
					}
					const auto event = item->payload.event_id;
					const auto sequence = item->payload.sequence;
					const Key key{item->payload.time, event, sequence};
					const auto bytes = item->bytes;
					if (!pending.emplace(key, std::move(*item)).second) {
						throw std::invalid_argument("Duplicate SRO payload ordering key");
					}
					pending_bytes += bytes;
					if (on_delivery) { on_delivery(event, sequence); }
				}
				else {
					const auto boundary = std::get<GSROTime>(message);
					consume_before(pending, pending_bytes, *resources.plugin, boundary);
					resources.plugin->advance_time(boundary);
					processed_safe_time = boundary;
				}
			}
			if (context.safe_time && (!processed_safe_time || *context.safe_time > *processed_safe_time)) {
				consume_before(pending, pending_bytes, *resources.plugin, context.safe_time);
				resources.plugin->advance_time(*context.safe_time);
			}
			consume_before(pending, pending_bytes, *resources.plugin, std::nullopt);
			rethrow_if_failed();
			resources.plugin->finish_run(context);
			rethrow_if_failed();
			resources.sink->finish_output();
		}
		catch (...) {
			// Destroy rejected payloads outside the mutex; data destructors belong to external implementations.
			std::deque<Message> rejected;
			std::exception_ptr error;
			{
				std::lock_guard lock(mutex);
				if (!failure) { failure = std::current_exception(); }
				error = failure;
				closing = true;
				rejected.swap(queue);
				queue_bytes = 0;
			}
			space_available.notify_all();
			if (on_failure) {
				try { on_failure(error); }
				catch (...) { /* Preserve the original background failure. */ }
			}
		}
	}

	const GSROCrateId crate_id;
	ResourceFactory make_resources;
	const GSROCrateLimits limits;
	DeliveryCallback on_delivery;
	FailureCallback on_failure;
	mutable std::mutex mutex;
	std::condition_variable input_available;
	std::condition_variable space_available;
	std::deque<Message> queue;
	std::size_t queue_bytes = 0;
	std::optional<GSROTime> submitted_safe_time;
	std::optional<GSROEndContext> end_context;
	std::exception_ptr failure;
	bool closing = false;
	jthread_alias thread;
};

GSROCrate::GSROCrate(GSROCrateId crate_id, ResourceFactory make_resources,
                     GSROCrateLimits limits, DeliveryCallback on_delivery, FailureCallback on_failure)
	: impl(std::make_unique<Impl>(crate_id, std::move(make_resources), limits,
	                              std::move(on_delivery), std::move(on_failure))) {}

GSROCrate::~GSROCrate() {
	try { impl->finish_and_join({GSROEndReason::interrupted, std::nullopt}); }
	catch (...) { /* Explicit finish_and_join is the error-reporting path. */ }
}

void GSROCrate::enqueue_payload(GSROPayload payload) { impl->enqueue_payload(std::move(payload)); }
void GSROCrate::advance_time(GSROTime safe_time) { impl->advance_time(safe_time); }
void GSROCrate::request_finish(GSROEndContext context) { impl->request_finish(context); }
void GSROCrate::cancel(std::exception_ptr error) { impl->cancel(error); }
void GSROCrate::finish_and_join(GSROEndContext context) { impl->finish_and_join(context); }
void GSROCrate::rethrow_if_failed() const { impl->rethrow_if_failed(); }
