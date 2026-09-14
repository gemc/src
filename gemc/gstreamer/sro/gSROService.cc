#include "gSROService.h"

#include <gemc/guts/gthreads.h>

#include <condition_variable>
#include <limits>
#include <map>
#include <mutex>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

class GSROService::Impl
{
public:
	void begin_run(ResourceFactory factory, GSROCrateLimits bounds, DeliveryCallback delivered,
	               std::shared_ptr<const GSROTiming> model = {}, std::size_t event_limit = 65536) {
		if (!factory || !bounds.queue_messages || !bounds.queue_bytes ||
		    !bounds.pending_payloads || !bounds.pending_bytes || !event_limit) {
			throw std::invalid_argument("SRO run requires a resource factory and positive buffer limits");
		}
		std::lock_guard lock(mutex);
		if (state == State::running || state == State::finishing) {
			throw std::logic_error("Finish the previous SRO run before beginning another");
		}
		crates.clear(); // All previous threads have already been joined.
		make_resources = std::move(factory);
		on_delivery = std::move(delivered);
		limits = bounds;
		safe_time.reset();
		failure = nullptr;
		timing = std::move(model);
		max_pending_events = event_limit;
		events.clear();
		first_undelivered_event = 0;
		outstanding_payloads = 0;
		progress_dirty = true;
		stop_progress = false;
		state = State::running;
		try {
			if (timing) { progress_thread = jthread_alias([this] { run_progress(); }); }
		}
		catch (...) {
			state = State::finished;
			failure = std::current_exception();
			throw;
		}
	}

	std::shared_ptr<GSROCrate> create_crate_thread_if_needed(GSROCrateId id) {
		std::unique_lock lock(mutex);
		check_running();
		const auto found = crates.find(id);
		if (found != crates.end()) { return found->second; }
		// Allocate the registry slot before starting a thread; insertion failure must not join under this lock.
		auto [entry, inserted] = crates.try_emplace(id);
		try {
			entry->second = std::make_shared<GSROCrate>(id,
				[factory = make_resources, id] { return factory(id); }, limits,
				[this, delivered = on_delivery, id](GSROEventId event, std::uint64_t sequence) {
					if (delivered) { delivered(id, event, sequence); }
					acknowledge_payload(event);
				}, [this](std::exception_ptr error) { fail_run(error); });
			// This queue is new and empty; seeding its boundary cannot wait for queue space.
			if (safe_time) { entry->second->advance_time(*safe_time); }
			return entry->second;
		}
		catch (...) {
			if (!entry->second) { crates.erase(entry); }
			lock.unlock();
			fail_run(std::current_exception());
			throw;
		}
	}

	void dispatch_payload_to_crate(GSROPayload payload) {
		if (!payload.data) { throw std::invalid_argument("SRO payload data is null"); }
		{
			std::lock_guard lock(mutex);
			check_running();
			if (timing) {
				auto& event = open_event(payload.event_id);
				// Register before enqueue: the crate may acknowledge before this worker returns.
				++event.outstanding;
				++outstanding_payloads;
			}
		}
		try {
			auto crate = create_crate_thread_if_needed(payload.crate_id);
			// The local shared_ptr keeps the crate alive if shutdown races with this dispatch.
			crate->enqueue_payload(std::move(payload));
		}
		catch (...) {
			const auto error = std::current_exception();
			std::lock_guard lock(mutex);
			// An in-flight producer woken by normal shutdown is rejected, without cancelling accepted data.
			if (state == State::running) { fail_run_locked(error); }
			std::rethrow_exception(failure ? failure : error);
		}
	}

	void complete_event(GSROEventId id) {
		std::lock_guard lock(mutex);
		check_running();
		if (!timing) { throw std::logic_error("SRO event completion requires automatic timing"); }
		open_event(id).closed = true;
		update_prefix();
	}

	void advance_time(GSROTime boundary) {
		{
			std::lock_guard lock(mutex);
			check_running();
			if (timing) { throw std::logic_error("SRO automatic timing owns safe-time advancement"); }
		}
		broadcast_time(boundary);
	}

	void broadcast_time(GSROTime boundary) {
		std::vector<std::shared_ptr<GSROCrate>> snapshot;
		{
			std::lock_guard lock(mutex);
			if (failure) { std::rethrow_exception(failure); }
			if (safe_time && boundary < *safe_time) {
				throw std::invalid_argument("SRO service safe-time boundaries must not decrease");
			}
			safe_time = boundary;
			for (const auto& [id, crate] : crates) { snapshot.push_back(crate); }
		}
		try {
			for (const auto& crate : snapshot) { crate->advance_time(boundary); }
		}
		catch (...) {
			fail_run(std::current_exception());
			rethrow_if_failed();
		}
	}

	void finish_run(GSROEndContext context) {
		std::vector<std::shared_ptr<GSROCrate>> snapshot;
		{
			std::unique_lock lock(mutex);
			if (state == State::idle || state == State::finished) {
				if (failure) { std::rethrow_exception(failure); }
				return;
			}
			if (timing) {
				if (context.safe_time) {
					throw std::invalid_argument("SRO automatic timing owns the final safe-time boundary");
				}
				state = State::finishing;
				// Workers have returned. Keep crate inputs open while the last acknowledgments and
				// progress messages run; closing them now would lose the final proven prefix.
				progress_changed.wait(lock, [&] { return failure || outstanding_payloads == 0; });
				stop_progress = true;
				progress_changed.notify_all();
				lock.unlock();
				if (progress_thread.joinable()) { progress_thread.join(); }
				lock.lock();
			}
			if (context.safe_time && safe_time && *context.safe_time < *safe_time) {
				throw std::invalid_argument("SRO final boundary precedes the service boundary");
			}
			if (!context.safe_time) { context.safe_time = safe_time; }
			state = State::finishing;
			for (const auto& [id, crate] : crates) {
				crate->request_finish(context);
				snapshot.push_back(crate);
			}
		}
		// All producer waiters have been woken before any join. A failed crate must not skip other joins.
		for (const auto& crate : snapshot) {
			try { crate->finish_and_join(context); }
			catch (...) { fail_run(std::current_exception()); }
		}
		{
			std::lock_guard lock(mutex);
			state = State::finished;
		}
		rethrow_if_failed();
	}

	void rethrow_if_failed() const {
		std::lock_guard lock(mutex);
		if (failure) { std::rethrow_exception(failure); }
	}

	void cancel_run(std::exception_ptr error) {
		if (!error) { throw std::invalid_argument("SRO cancellation requires an exception"); }
		fail_run(error);
	}

private:
	struct Event {
		std::size_t outstanding = 0;
		bool closed = false;
	};

	Event& open_event(GSROEventId id) {
		if (id == std::numeric_limits<GSROEventId>::max()) {
			throw std::invalid_argument("SRO event ID would overflow the completion prefix");
		}
		if (id < first_undelivered_event) { throw std::logic_error("SRO event is already complete"); }
		auto found = events.find(id);
		if (found == events.end()) {
			if (events.size() >= max_pending_events) {
				const auto error = std::make_exception_ptr(
					std::length_error("SRO pending event limit exceeded"));
				fail_run_locked(error);
				std::rethrow_exception(error);
			}
			found = events.try_emplace(id).first;
		}
		if (found->second.closed) { throw std::logic_error("SRO event is already complete"); }
		return found->second;
	}

	void acknowledge_payload(GSROEventId id) {
		std::lock_guard lock(mutex);
		if (!timing) { return; }
		auto& event = events.at(id);
		--event.outstanding;
		--outstanding_payloads;
		update_prefix();
		progress_changed.notify_all();
	}

	void update_prefix() {
		while (!events.empty()) {
			const auto next = events.begin();
			if (next->first != first_undelivered_event || !next->second.closed || next->second.outstanding) {
				break;
			}
			events.erase(next);
			++first_undelivered_event;
			progress_dirty = true;
		}
		if (progress_dirty) { progress_changed.notify_all(); }
	}

	void run_progress() noexcept {
		try {
			for (;;) {
				GSROEventId prefix;
				{
					std::unique_lock lock(mutex);
					progress_changed.wait(lock, [&] { return failure || stop_progress || progress_dirty; });
					if (failure || (stop_progress && !progress_dirty)) { return; }
					prefix = first_undelivered_event;
					progress_dirty = false;
				}
				// Neither implementation calls nor queue-space waits hold the registry mutex.
				if (const auto boundary = timing->earliest_remaining_time(prefix)) {
					broadcast_time(*boundary);
				}
			}
		}
		catch (...) { fail_run(std::current_exception()); }
	}

	void check_running() const {
		if (failure) { std::rethrow_exception(failure); }
		if (state != State::running) { throw std::logic_error("SRO run is not accepting input"); }
	}

	void fail_run(std::exception_ptr error) {
		std::lock_guard lock(mutex);
		fail_run_locked(error);
	}

	void fail_run_locked(std::exception_ptr error) {
		if (failure) { return; }
		failure = error;
		progress_changed.notify_all();
		// cancel only takes a crate's queue mutex. Crate failure notifications never hold that mutex.
		for (const auto& [id, crate] : crates) { crate->cancel(error); }
	}

	enum class State { idle, running, finishing, finished };
	mutable std::mutex mutex;
	State state = State::idle;
	ResourceFactory make_resources;
	DeliveryCallback on_delivery;
	GSROCrateLimits limits;
	std::unordered_map<GSROCrateId, std::shared_ptr<GSROCrate>> crates;
	std::optional<GSROTime> safe_time;
	std::exception_ptr failure;
	std::shared_ptr<const GSROTiming> timing;
	std::size_t max_pending_events = 65536;
	std::map<GSROEventId, Event> events;
	GSROEventId first_undelivered_event = 0;
	std::size_t outstanding_payloads = 0;
	bool progress_dirty = false;
	bool stop_progress = false;
	std::condition_variable progress_changed;
	jthread_alias progress_thread;
};

GSROService::GSROService() : impl(std::make_unique<Impl>()) {}
GSROService::~GSROService() {
	try { impl->finish_run({GSROEndReason::interrupted, std::nullopt}); }
	catch (...) { /* Explicit finish_run reports failures. All crate threads have still been joined. */ }
}
void GSROService::begin_run(ResourceFactory factory, GSROCrateLimits limits, DeliveryCallback delivered) {
	impl->begin_run(std::move(factory), limits, std::move(delivered));
}
void GSROService::begin_run(ResourceFactory factory, std::shared_ptr<const GSROTiming> timing,
                           GSROCrateLimits limits, std::size_t max_pending_events) {
	if (!timing) { throw std::invalid_argument("SRO automatic timing model is null"); }
	impl->begin_run(std::move(factory), limits, {}, std::move(timing), max_pending_events);
}
void GSROService::create_crate_thread_if_needed(GSROCrateId id) { impl->create_crate_thread_if_needed(id); }
void GSROService::dispatch_payload_to_crate(GSROPayload payload) {
	impl->dispatch_payload_to_crate(std::move(payload));
}
void GSROService::advance_time(GSROTime safe_time) { impl->advance_time(safe_time); }
void GSROService::complete_event(GSROEventId id) { impl->complete_event(id); }
void GSROService::finish_run(GSROEndContext context) { impl->finish_run(context); }
void GSROService::rethrow_if_failed() const { impl->rethrow_if_failed(); }
void GSROService::cancel_run(std::exception_ptr error) { impl->cancel_run(error); }
