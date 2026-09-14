#include "sro/gSROService.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <future>
#include <iostream>
#include <limits>
#include <map>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>

using namespace std::chrono_literals;

namespace {
void require(bool condition, const char* message) {
	if (!condition) { throw std::runtime_error(message); }
}

template<class F>
void expect_error(F&& action, const std::string& message) {
	try { action(); }
	catch (const std::exception& error) {
		require(std::string(error.what()).find(message) != std::string::npos, "Unexpected service error");
		return;
	}
	throw std::runtime_error("Expected an exception: " + message);
}

std::atomic<int> live_data{0};
struct Data final : GSROData {
	Data() { ++live_data; }
	~Data() override { --live_data; }
	std::size_t size_bytes() const noexcept override { return sizeof(*this); }
};

GSROPayload payload(GSROCrateId crate, GSROEventId event, std::int64_t time = 1) {
	return {crate, event, 0, GSROTime{time}, std::make_unique<Data>()};
}

using Key = std::tuple<GSROTime, GSROEventId, std::uint64_t>;
struct CrateState {
	int created = 0;
	int destroyed = 0;
	int closed = 0;
	int delivered = 0;
	std::thread::id owner;
	std::vector<Key> consumed;
	std::vector<GSROTime> boundaries;
	std::optional<GSROEndContext> end;
};

struct State {
	std::mutex mutex;
	std::condition_variable changed;
	std::map<GSROCrateId, CrateState> crates;
	std::optional<GSROCrateId> fail_close;
	bool wrong_thread = false;

	template<class F>
	void update(GSROCrateId id, F&& action) {
		std::lock_guard lock(mutex);
		auto& crate = crates.at(id);
		wrong_thread |= crate.owner != std::this_thread::get_id();
		action(crate);
		changed.notify_all();
	}

	template<class F>
	void wait_for(F&& condition) {
		std::unique_lock lock(mutex);
		require(changed.wait_for(lock, 5s, condition), "Timed out waiting for crate progress");
	}
};

struct Sink final : GSROFrameSink {
	Sink(GSROCrateId crate, std::shared_ptr<State> s) : id(crate), state(std::move(s)) {}
	~Sink() override { state->update(id, [](auto& c) { ++c.destroyed; }); }
	void write_frame(GSROFrame) override {}
	void finish_output() override {
		state->update(id, [](auto& c) { ++c.closed; });
		if (state->fail_close == id) { throw std::runtime_error("synthetic close failure"); }
	}
	GSROCrateId id;
	std::shared_ptr<State> state;
};

struct Plugin final : GSROCratePlugin {
	Plugin(GSROFrameSink& sink, GSROCrateId crate, std::shared_ptr<State> s)
		: GSROCratePlugin(sink), id(crate), state(std::move(s)) {}
	void consume_payload(GSROPayload item) override {
		require(item.crate_id == id, "Service misrouted a payload");
		state->update(id, [&](auto& c) { c.consumed.emplace_back(item.time, item.event_id, item.sequence); });
	}
	void advance_time(GSROTime time) override {
		state->update(id, [&](auto& c) { c.boundaries.push_back(time); });
	}
	void finish_run(const GSROEndContext& context) override {
		state->update(id, [&](auto& c) { c.end = context; });
	}
	GSROCrateId id;
	std::shared_ptr<State> state;
};

GSROService::ResourceFactory factory(const std::shared_ptr<State>& state,
                                     std::function<void(GSROCrateId)> before = {}) {
	return [state, before](GSROCrateId id) {
		{
			std::lock_guard lock(state->mutex);
			auto& crate = state->crates[id];
			++crate.created;
			crate.owner = std::this_thread::get_id();
		}
		state->changed.notify_all();
		if (before) { before(id); }
		auto sink = std::make_unique<Sink>(id, state);
		auto plugin = std::make_unique<Plugin>(*sink, id, state);
		return GSROCrateResources{std::move(sink), std::move(plugin)};
	};
}

void test_routing_and_runs() {
	GSROService service;
	expect_error([&] { service.create_crate_thread_if_needed(1); }, "not accepting");
	expect_error([&] { service.begin_run({}); }, "resource factory");
	service.finish_run({GSROEndReason::completed, std::nullopt});
	auto state = std::make_shared<State>();
	service.begin_run(factory(state), {}, [state](GSROCrateId id, GSROEventId, std::uint64_t) {
		state->update(id, [](auto& c) { ++c.delivered; });
	});
	require(state->crates.empty(), "begin_run eagerly created crates");
	expect_error([&] { service.begin_run(factory(state)); }, "previous SRO run");
	std::vector<std::future<void>> workers;
	for (GSROEventId worker = 0; worker < 8; ++worker) {
		workers.push_back(std::async(std::launch::async, [&, worker] {
			for (int n = 31; n >= 0; --n) {
				service.create_crate_thread_if_needed(7);
				service.dispatch_payload_to_crate(payload(7 + n % 2, worker * 32 + n, n % 4));
			}
		}));
	}
	for (auto& worker : workers) { worker.get(); }
	service.advance_time(GSROTime{10});
	// A crate appearing after progress must inherit the boundary before receiving any payload.
	service.dispatch_payload_to_crate(payload(9, 0, 10));
	service.create_crate_thread_if_needed(10); // Explicit empty-crate output.
	expect_error([&] { service.advance_time(GSROTime{9}); }, "must not decrease");
	expect_error([&] { service.finish_run({GSROEndReason::completed, GSROTime{9}}); }, "precedes");
	service.finish_run({GSROEndReason::completed, GSROTime{11}});
	service.finish_run({GSROEndReason::completed, std::nullopt});
	require(state->crates.size() == 4 && !state->wrong_thread, "Unexpected crate ownership");
	for (const auto& [id, c] : state->crates) {
		require(c.created == 1 && c.destroyed == 1 && c.closed == 1, "Crate created or closed more than once");
		require(c.owner != std::this_thread::get_id(), "Resource factory ran on the caller");
		require(c.end->safe_time == GSROTime{11}, "Crates did not share final progress");
		require(std::is_sorted(c.consumed.begin(), c.consumed.end()), "Crate delivery is out of order");
		require(c.boundaries == std::vector<GSROTime>{GSROTime{10}, GSROTime{11}},
		        "Lost inherited boundary");
		require(c.delivered == (id < 9 ? 128 : id == 9 ? 1 : 0), "Incorrect delivery acknowledgments");
		require(c.consumed.size() == static_cast<std::size_t>(c.delivered), "Payload lost during routing");
	}
	expect_error([&] { service.dispatch_payload_to_crate(payload(7, 0)); }, "not accepting");
	state = std::make_shared<State>();
	service.begin_run(factory(state));
	// Event IDs and simulation time restart per invocation.
	service.dispatch_payload_to_crate(payload(7, 0, -5));
	service.finish_run({GSROEndReason::completed, std::nullopt});
	require(state->crates.at(7).consumed.size() == 1 && !state->crates.at(7).end->safe_time,
	        "A new run inherited stale crate state");
}

void test_slow_crate(bool fail_other_crate) {
	GSROService service;
	auto state = std::make_shared<State>();
	std::promise<void> release;
	auto gate = release.get_future().share();
	GSROCrateLimits limits;
	limits.queue_messages = 1;
	service.begin_run(factory(state, [=](GSROCrateId id) {
		if (id == 1) { gate.wait(); }
		if (id == 2 && fail_other_crate) { throw std::runtime_error("synthetic setup failure"); }
	}), limits);
	service.dispatch_payload_to_crate(payload(1, 0));
	std::promise<void> started;
	auto ready = started.get_future();
	auto producer = std::async(std::launch::async, [&] {
		started.set_value();
		try { service.dispatch_payload_to_crate(payload(1, 1)); }
		catch (const std::exception& error) { return std::string(error.what()); }
		return std::string{};
	});
	ready.wait();
	const bool blocked = producer.wait_for(30ms) == std::future_status::timeout;
	// This dispatch must proceed while crate 1's producer is waiting for space.
	try { service.dispatch_payload_to_crate(payload(2, 2)); }
	catch (const std::runtime_error&) { /* Setup may fail before the initial dispatch returns. */ }
	std::future<void> finishing;
	if (!fail_other_crate) {
		finishing = std::async(std::launch::async, [&] {
			service.finish_run({GSROEndReason::interrupted, std::nullopt});
		});
	}
	const bool woke = producer.wait_for(5s) == std::future_status::ready;
	release.set_value();
	const auto error = producer.get();
	require(blocked && woke, "Cross-crate progress, cancellation, or shutdown deadlocked");
	if (fail_other_crate) {
		require(error.find("synthetic setup failure") != std::string::npos,
		        "Peer failure did not wake producer");
		expect_error([&] { service.finish_run({GSROEndReason::completed, std::nullopt}); }, "setup failure");
		expect_error([&] { service.rethrow_if_failed(); }, "setup failure");
		expect_error([&] { service.create_crate_thread_if_needed(3); }, "setup failure");
		// Recovery requires joining the failed run, then starting a fresh invocation.
		state = std::make_shared<State>();
		service.begin_run(factory(state));
		service.dispatch_payload_to_crate(payload(3, 0));
		service.finish_run({GSROEndReason::completed, std::nullopt});
		require(state->crates.at(3).closed == 1, "Failed run poisoned the following run");
	}
	else {
		finishing.get();
		require(error.find("closed") != std::string::npos, "Shutdown did not reject blocked submission");
		require(state->crates.at(1).consumed.size() == 1 && state->crates.at(2).consumed.size() == 1,
		        "Normal shutdown cancelled previously accepted data");
	}
}

void test_close_failure_and_destructor() {
	auto state = std::make_shared<State>();
	state->fail_close = 1;
	{
		GSROService service;
		service.begin_run(factory(state));
		service.dispatch_payload_to_crate(payload(1, 0));
		service.dispatch_payload_to_crate(payload(2, 0));
		expect_error([&] { service.finish_run({GSROEndReason::completed, std::nullopt}); }, "close failure");
		require(state->crates.at(1).destroyed == 1 && state->crates.at(2).destroyed == 1,
		        "One close failure prevented another crate from joining");
	}
	state = std::make_shared<State>();
	{
		GSROService service;
		service.begin_run(factory(state));
		service.dispatch_payload_to_crate(payload(4, 0));
	}
	require(state->crates.at(4).end->reason == GSROEndReason::interrupted && state->crates.at(4).destroyed == 1,
	        "Service destructor did not finalize and join");
}

struct Timing final : GSROTiming {
	std::function<std::optional<GSROTime>(GSROEventId)> bound = [](GSROEventId id) {
		return GSROTime{static_cast<std::int64_t>(id) * 10 - 10};
	};
	std::optional<GSROTime> earliest_remaining_time(GSROEventId id) const override { return bound(id); }
};

void test_automatic_delivery() {
	GSROService service;
	auto state = std::make_shared<State>();
	auto timing = std::make_shared<Timing>();
	std::promise<void> release;
	auto gate = release.get_future().share();
	service.begin_run(factory(state, [=](GSROCrateId id) { if (id == 2) { gate.wait(); } }), timing);
	service.dispatch_payload_to_crate(payload(1, 0, -5));
	auto second = payload(2, 0, -10);
	second.sequence = 1;
	service.dispatch_payload_to_crate(std::move(second));
	service.complete_event(1); // Empty event finishes before event zero.
	service.complete_event(0); // Closed, but crate 2 has not retained its payload yet.
	state->wait_for([&] { return state->crates.count(1) && !state->crates.at(1).boundaries.empty(); });
	bool held;
	{
		std::lock_guard lock(state->mutex);
		held = state->crates.at(1).boundaries.back() == GSROTime{-10} &&
		       state->crates.at(1).consumed.empty();
	}
	release.set_value();
	require(held, "Closed event advanced before every crate acknowledged delivery");
	state->wait_for([&] { return state->crates.at(1).boundaries.back() == GSROTime{10}; });
	// A new crate inherits progress. Equality stays pending until another complete event advances time.
	service.dispatch_payload_to_crate(payload(3, 2, 10));
	state->wait_for([&] {
		const auto found = state->crates.find(3);
		return found != state->crates.end() && !found->second.boundaries.empty();
	});
	{
		std::lock_guard lock(state->mutex);
		require(state->crates.at(3).consumed.empty(), "Equal-boundary payload released too early");
	}
	expect_error([&] { service.complete_event(1); }, "already complete");
	expect_error([&] { service.dispatch_payload_to_crate(payload(1, 0)); }, "already complete");
	expect_error([&] { service.advance_time(GSROTime{100}); }, "automatic timing");
	expect_error([&] { service.finish_run({GSROEndReason::completed, GSROTime{100}}); }, "automatic timing");
	service.complete_event(2);
	service.finish_run({GSROEndReason::completed, std::nullopt});
	for (const auto& [id, crate] : state->crates) {
		require(crate.consumed.size() == 1 && crate.end->safe_time == GSROTime{20},
		        "Final acknowledgment or automatic boundary was lost");
	}
}

void test_automatic_workers() {
	GSROService service;
	auto state = std::make_shared<State>();
	GSROCrateLimits limits;
	limits.queue_messages = 2; // Exercise progress broadcasts while producers wait for queue space.
	service.begin_run(factory(state), std::make_shared<Timing>(), limits);
	std::vector<std::future<void>> workers;
	for (GSROEventId worker = 0; worker < 8; ++worker) {
		workers.push_back(std::async(std::launch::async, [&, worker] {
			for (int n = 31; n >= 0; --n) {
				const auto event = worker * 32 + n;
				if (event % 4) {
					service.dispatch_payload_to_crate(payload(7, event, event * 10 - 5));
					auto second = payload(8, event, event * 10 - 10);
					second.sequence = 1;
					service.dispatch_payload_to_crate(std::move(second));
				}
				service.complete_event(event);
			}
		}));
	}
	for (auto& worker : workers) { worker.get(); }
	service.finish_run({GSROEndReason::completed, std::nullopt});
	for (const auto& [id, crate] : state->crates) {
		require(crate.consumed.size() == 192, "Concurrent automatic dispatch lost payloads");
		require(std::is_sorted(crate.consumed.begin(), crate.consumed.end()),
		        "Automatic delivery out of order");
		require(crate.end->safe_time == GSROTime{2550}, "Empty or reordered events left incorrect progress");
	}
	require(!state->wrong_thread, "Automatic progress bypassed crate thread ownership");
	// A subsequent invocation starts at event zero, with no inherited completion or timing state.
	state = std::make_shared<State>();
	service.begin_run(factory(state), std::make_shared<Timing>());
	service.dispatch_payload_to_crate(payload(1, 0, -10));
	service.complete_event(0);
	service.finish_run({GSROEndReason::completed, std::nullopt});
	require(state->crates.at(1).end->safe_time == GSROTime{0}, "Automatic state leaked across runs");
}

void test_missing_events_and_unknown_time() {
	for (int mode = 0; mode < 3; ++mode) {
		auto state = std::make_shared<State>();
		{
			GSROService service;
			auto timing = std::make_shared<Timing>();
			if (mode == 2) { timing->bound = [](GSROEventId) { return std::nullopt; }; }
			service.begin_run(factory(state), timing);
			service.dispatch_payload_to_crate(payload(1, 1, 15));
			service.complete_event(1);
			if (mode == 1) {
				service.dispatch_payload_to_crate(payload(1, 0, -5)); // Delivered but never closed.
			}
			// Destructor drains accepted payloads but must not complete missing/unfinished event zero.
		}
		const auto& crate = state->crates.at(1);
		require(crate.end->reason == GSROEndReason::interrupted, "Interruption was lost");
		require(crate.end->safe_time == (mode == 2 ? std::nullopt : std::optional<GSROTime>{GSROTime{-10}}),
		        "Shutdown invented progress across a missing event or unknown time");
		require(crate.consumed.size() == (mode == 1 ? 2u : 1u), "Interrupted shutdown lost accepted payloads");
	}
	// All-empty runs still query the timing implementation through the final contiguous prefix.
	GSROService service;
	auto state = std::make_shared<State>();
	auto timing = std::make_shared<Timing>();
	std::atomic<GSROEventId> last{0};
	timing->bound = [&](GSROEventId id) { last = id; return std::nullopt; };
	service.begin_run(factory(state), timing);
	service.complete_event(2);
	service.complete_event(0);
	service.complete_event(1);
	service.finish_run({GSROEndReason::completed, std::nullopt});
	require(last == 3 && state->crates.empty(), "All-empty run lost completion or created an output crate");
}

void test_automatic_failures() {
	GSROService service;
	auto state = std::make_shared<State>();
	auto timing = std::make_shared<Timing>();
	expect_error([&] { service.begin_run(factory(state), std::shared_ptr<const GSROTiming>{}); }, "null");
	expect_error([&] { service.begin_run(factory(state), timing, {}, 0); }, "positive");
	service.begin_run(factory(state), timing, {}, 2);
	expect_error([&] { service.complete_event(std::numeric_limits<GSROEventId>::max()); }, "overflow");
	service.complete_event(1);
	service.complete_event(2);
	expect_error([&] { service.complete_event(3); }, "pending event limit");
	expect_error([&] { service.finish_run({GSROEndReason::completed, std::nullopt}); }, "pending event limit");
	for (bool throws : {false, true}) {
		state = std::make_shared<State>();
		timing = std::make_shared<Timing>();
		timing->bound = [=](GSROEventId id) {
			if (id && throws) { throw std::runtime_error("synthetic timing failure"); }
			return GSROTime{id ? -20 : -10};
		};
		service.begin_run(factory(state), timing);
		service.create_crate_thread_if_needed(1);
		state->wait_for([&] { return state->crates.count(1) && !state->crates.at(1).boundaries.empty(); });
		service.complete_event(0);
		expect_error([&] { service.finish_run({GSROEndReason::completed, std::nullopt}); },
		             throws ? "timing failure" : "must not decrease");
		require(state->crates.at(1).destroyed == 1, "Timing failure skipped crate cleanup");
	}
	// A failed enqueue was registered before submission; shutdown must fail, not wait for a missing ack.
	state = std::make_shared<State>();
	GSROCrateLimits limits;
	limits.queue_bytes = sizeof(GSROPayload);
	service.begin_run(factory(state), std::make_shared<Timing>(), limits);
	expect_error([&] { service.dispatch_payload_to_crate(payload(1, 0)); }, "byte limit");
	expect_error([&] { service.finish_run({GSROEndReason::interrupted, std::nullopt}); }, "byte limit");
}

void test_automatic_waiters(bool fail_timing) {
	GSROService service;
	auto state = std::make_shared<State>();
	auto timing = std::make_shared<Timing>();
	timing->bound = [=](GSROEventId id) -> std::optional<GSROTime> {
		if (!id) { return std::nullopt; } // No initial control message occupies the gated input queue.
		if (fail_timing) { throw std::runtime_error("synthetic timing failure"); }
		return GSROTime{static_cast<std::int64_t>(id) * 10};
	};
	std::promise<void> release;
	auto gate = release.get_future().share();
	GSROCrateLimits limits;
	limits.queue_messages = 1;
	service.begin_run(factory(state, [=](GSROCrateId) { gate.wait(); }), timing, limits);
	service.dispatch_payload_to_crate(payload(1, fail_timing ? 1 : 0));
	if (fail_timing) {
		std::promise<void> started;
		auto ready = started.get_future();
		auto producer = std::async(std::launch::async, [&] {
			started.set_value();
			try { service.dispatch_payload_to_crate(payload(1, 2)); }
			catch (const std::exception& error) { return std::string(error.what()); }
			return std::string{};
		});
		ready.wait();
		const bool blocked = producer.wait_for(30ms) == std::future_status::timeout;
		service.complete_event(0); // Timing failure must cancel the crate and wake its blocked worker.
		const bool woke = producer.wait_for(5s) == std::future_status::ready;
		release.set_value();
		const auto error = producer.get();
		require(blocked && woke && error.find("timing failure") != std::string::npos,
		        "Timing failure did not wake a worker blocked on a full crate queue");
		expect_error([&] { service.finish_run({GSROEndReason::interrupted, std::nullopt}); }, "timing failure");
	}
	else {
		service.complete_event(0);
		auto finishing = std::async(std::launch::async, [&] {
			service.finish_run({GSROEndReason::completed, std::nullopt});
		});
		const bool waiting = finishing.wait_for(30ms) == std::future_status::timeout;
		release.set_value();
		finishing.get();
		require(waiting && state->crates.at(1).end->safe_time == GSROTime{10},
		        "Shutdown failed to wait for the last delivery and derive its final bound");
	}
}
} // namespace

int main() {
	try {
		test_routing_and_runs();
		test_slow_crate(false);
		test_slow_crate(true);
		test_close_failure_and_destructor();
		test_automatic_delivery();
		test_automatic_workers();
		test_missing_events_and_unknown_time();
		test_automatic_failures();
		test_automatic_waiters(false);
		test_automatic_waiters(true);
		require(live_data == 0, "Service leaked payloads");
		std::cout << "SRO service routing, automatic progress, shutdown, and failure checks passed\n";
		return 0;
	}
	catch (const std::exception& error) {
		std::cerr << error.what() << '\n';
		return 1;
	}
}
