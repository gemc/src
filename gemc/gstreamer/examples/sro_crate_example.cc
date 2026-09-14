#include "sro/gSROCrate.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <future>
#include <iostream>
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
		require(std::string(error.what()).find(message) != std::string::npos, "Unexpected exception");
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

GSROPayload payload(std::int64_t time, GSROEventId event = 0, std::uint64_t sequence = 0) {
	return {7, event, sequence, GSROTime{time}, std::make_unique<Data>()};
}

using Key = std::tuple<GSROTime, GSROEventId, std::uint64_t>;
struct State {
	std::mutex mutex;
	std::condition_variable changed;
	std::thread::id owner;
	bool wrong_thread = false;
	std::vector<Key> consumed;
	std::vector<std::string> lifecycle;
	std::optional<GSROTime> boundary;
	std::optional<GSROEndContext> end;
	std::atomic<int> delivered{0};
	std::string fail_at;

	void record(const std::string& operation) {
		std::lock_guard lock(mutex);
		wrong_thread |= owner != std::this_thread::get_id();
		lifecycle.push_back(operation);
	}
	void maybe_fail(const std::string& operation) const {
		if (fail_at == operation) { throw std::runtime_error("synthetic " + operation + " failure"); }
	}
};

struct Sink final : GSROFrameSink {
	explicit Sink(std::shared_ptr<State> s) : state(std::move(s)) { state->record("sink-created"); }
	~Sink() override { state->record("sink-destroyed"); }
	void write_frame(GSROFrame frame) override {
		require(frame.crate_id == 7 && frame.data != nullptr, "Invalid emitted frame");
		state->record("write");
		state->maybe_fail("write");
	}
	void finish_output() override {
		state->record("close");
		state->maybe_fail("close");
	}
	std::shared_ptr<State> state;
};

// Deliberately simple implementation: retain contributions until advance_time; discard its final partial data.
struct Plugin final : GSROCratePlugin {
	Plugin(GSROFrameSink& sink, std::shared_ptr<State> s) : GSROCratePlugin(sink), state(std::move(s)) {
		state->record("plugin-created");
	}
	~Plugin() override { state->record("plugin-destroyed"); }
	void consume_payload(GSROPayload item) override {
		state->record("consume");
		state->maybe_fail("consume");
		{
			std::lock_guard lock(state->mutex);
			state->consumed.emplace_back(item.time, item.event_id, item.sequence);
		}
		pending.push_back(std::move(item));
	}
	void advance_time(GSROTime time) override {
		state->record("advance");
		state->maybe_fail("advance");
		for (auto& item : pending) {
			output.write_frame({item.crate_id, item.sequence, item.time, time, std::move(item.data)});
		}
		pending.clear();
		{
			std::lock_guard lock(state->mutex);
			state->boundary = time;
		}
		state->changed.notify_all();
	}
	void finish_run(const GSROEndContext& context) override {
		state->record("finish");
		state->maybe_fail("finish");
		state->end = context;
		pending.clear();
	}
	std::shared_ptr<State> state;
	std::vector<GSROPayload> pending;
};

GSROCrate::ResourceFactory factory(const std::shared_ptr<State>& state) {
	return [state] {
		state->owner = std::this_thread::get_id();
		state->maybe_fail("setup");
		auto sink = std::make_unique<Sink>(state);
		auto plugin = std::make_unique<Plugin>(*sink, state);
		return GSROCrateResources{std::move(sink), std::move(plugin)};
	};
}

void test_ordering_and_lifecycle() {
	auto state = std::make_shared<State>();
	GSROCrate crate(7, factory(state), {}, [state](GSROEventId, std::uint64_t) { ++state->delivered; });
	std::vector<std::future<void>> workers;
	std::vector<Key> expected;
	for (GSROEventId event = 0; event < 4; ++event) {
		for (int seq = 0; seq < 32; ++seq) { expected.emplace_back(GSROTime{seq % 5 - 2}, event, seq); }
		workers.push_back(std::async(std::launch::async, [&, event] {
			for (int seq = 31; seq >= 0; --seq) { crate.enqueue_payload(payload(seq % 5 - 2, event, seq)); }
		}));
	}
	for (auto& worker : workers) { worker.get(); }
	crate.advance_time(GSROTime{0});
	{
		std::unique_lock lock(state->mutex);
		require(state->changed.wait_for(lock, 5s, [&] { return state->boundary == GSROTime{0}; }),
		        "Crate did not process its boundary");
		require(state->consumed.size() == 56, "Boundary equality or negative-time ordering is incorrect");
		for (const auto& key : state->consumed) { require(std::get<0>(key) < GSROTime{0}, "Early release"); }
	}
	crate.advance_time(GSROTime{0});
	expect_error([&] { crate.advance_time(GSROTime{-1}); }, "must not decrease");
	expect_error([&] { crate.enqueue_payload(payload(-1)); }, "precedes");
	expect_error([&] { crate.finish_and_join({GSROEndReason::completed, GSROTime{-1}}); }, "precedes");
	crate.finish_and_join({GSROEndReason::completed, GSROTime{2}});
	crate.finish_and_join({GSROEndReason::completed, std::nullopt});
	std::sort(expected.begin(), expected.end());
	require(state->consumed == expected && state->delivered == 128, "Lost, duplicated, or reordered payload");
	require(state->end->safe_time == GSROTime{2}, "Final progress was lost");
	require(!state->wrong_thread && state->owner != std::this_thread::get_id(), "Wrong callback thread");
	const std::vector<std::string> ending{"finish", "close", "plugin-destroyed", "sink-destroyed"};
	require(std::equal(ending.begin(), ending.end(), state->lifecycle.end() - 4), "Wrong shutdown order");
	require(std::count(state->lifecycle.begin(), state->lifecycle.end(), "write") == 104,
	        "Implementation did not retain and discard the final partial input");
	expect_error([&] { crate.enqueue_payload(payload(3)); }, "closed");
}

void test_blocked_producer(bool fail_setup, bool close_while_blocked, bool bound_by_bytes = false) {
	auto state = std::make_shared<State>();
	std::promise<void> release;
	auto gate = release.get_future().share();
	auto setup = factory(state);
	GSROCrateLimits limits;
	if (bound_by_bytes) { limits.queue_bytes = sizeof(GSROPayload) + sizeof(Data); }
	else { limits.queue_messages = 1; }
	GSROCrate crate(7, [=] {
		gate.wait();
		if (fail_setup) { throw std::runtime_error("synthetic setup failure"); }
		return setup();
	}, limits);
	crate.enqueue_payload(payload(1));
	std::promise<void> started;
	auto started_future = started.get_future();
	auto producer = std::async(std::launch::async, [&] {
		started.set_value();
		try { crate.enqueue_payload(payload(2)); }
		catch (const std::exception& error) { return std::string(error.what()); }
		return std::string{};
	});
	started_future.wait();
	const bool blocked = producer.wait_for(30ms) == std::future_status::timeout;
	std::future<void> finishing;
	if (close_while_blocked) {
		finishing = std::async(std::launch::async, [&] {
			crate.finish_and_join({GSROEndReason::interrupted, std::nullopt});
		});
	}
	const bool woke_on_close = !close_while_blocked || producer.wait_for(5s) == std::future_status::ready;
	release.set_value();
	const auto error = producer.get();
	if (finishing.valid()) { finishing.get(); }
	if (fail_setup) {
		expect_error([&] { crate.finish_and_join({GSROEndReason::completed, std::nullopt}); }, "setup failure");
		require(error.find("setup failure") != std::string::npos, "Blocked producer missed writer failure");
	}
	else {
		crate.finish_and_join({GSROEndReason::completed, std::nullopt});
		require(close_while_blocked ? error.find("closed") != std::string::npos : error.empty(),
		        "Unexpected blocked producer result");
		require(state->consumed.size() == (close_while_blocked ? 1u : 2u),
		        "Shutdown lost accepted input");
	}
	require(blocked && woke_on_close, "Queue backpressure or shutdown wakeup failed");
}

void test_failures_and_limits() {
	for (const std::string stage : {"setup", "consume", "advance", "write", "finish", "close", "delivery"}) {
		auto state = std::make_shared<State>();
		state->fail_at = stage;
		GSROCrate crate(7, factory(state), {}, [state](GSROEventId, std::uint64_t) {
			state->maybe_fail("delivery");
		});
		try {
			crate.enqueue_payload(payload(0));
			crate.advance_time(GSROTime{1});
		}
		catch (const std::runtime_error&) { /* An asynchronous failure can already reach a producer. */ }
		expect_error([&] { crate.finish_and_join({GSROEndReason::completed, std::nullopt}); },
		             "synthetic " + stage + " failure");
		expect_error([&] { crate.rethrow_if_failed(); }, "synthetic " + stage + " failure");
	}
	for (const bool byte_limit : {false, true}) {
		auto state = std::make_shared<State>();
		GSROCrateLimits limits;
		if (byte_limit) { limits.pending_bytes = sizeof(GSROPayload) + sizeof(Data); }
		else { limits.pending_payloads = 1; }
		GSROCrate crate(7, factory(state), limits);
		crate.enqueue_payload(payload(1));
		crate.enqueue_payload(payload(2));
		expect_error([&] { crate.finish_and_join({GSROEndReason::completed, std::nullopt}); },
		             "pending input limit");
	}
	auto state = std::make_shared<State>();
	GSROCrateLimits limits;
	limits.queue_bytes = 1;
	GSROCrate crate(7, factory(state), limits);
	expect_error([&] { crate.enqueue_payload(payload(0)); }, "byte limit");
	auto wrong_crate = payload(0);
	wrong_crate.crate_id = 8;
	expect_error([&] { crate.enqueue_payload(std::move(wrong_crate)); }, "another crate");
	expect_error([&] { crate.enqueue_payload({7, 0, 0, GSROTime{0}, nullptr}); }, "null");
	crate.finish_and_join({GSROEndReason::completed, std::nullopt});
	limits.queue_messages = 0;
	expect_error([&] { GSROCrate invalid(7, factory(state), limits); }, "positive buffer limits");
	GSROCrate invalid_resources(7, [] { return GSROCrateResources{}; });
	expect_error([&] { invalid_resources.finish_and_join({GSROEndReason::completed, std::nullopt}); },
	             "null plugin or sink");
	GSROCrate duplicates(7, factory(state));
	duplicates.enqueue_payload(payload(1));
	duplicates.enqueue_payload(payload(1));
	expect_error([&] { duplicates.finish_and_join({GSROEndReason::completed, std::nullopt}); }, "Duplicate");
}

void test_destructor_shutdown() {
	auto state = std::make_shared<State>();
	{
		GSROCrate crate(7, factory(state));
		crate.enqueue_payload(payload(1));
		crate.advance_time(GSROTime{1});
	}
	require(state->consumed.size() == 1 && state->end->reason == GSROEndReason::interrupted,
	        "Destructor did not drain and finalize");
	require(state->end->safe_time == GSROTime{1}, "Destructor lost the last safe boundary");
	require(!state->wrong_thread, "Destructor changed plugin ownership thread");
	state = std::make_shared<State>();
	state->fail_at = "close";
	{
		GSROCrate crate(7, factory(state));
	}
	require(state->lifecycle.back() == "sink-destroyed", "Destructor failed to clean up after close failure");
}
} // namespace

int main() {
	try {
		test_ordering_and_lifecycle();
		test_blocked_producer(false, false);
		test_blocked_producer(false, false, true);
		test_blocked_producer(false, true);
		test_blocked_producer(true, false);
		test_failures_and_limits();
		test_destructor_shutdown();
		require(live_data == 0, "Payload ownership leaked");
		std::cout << "SRO crate threading, ordering, limits, and lifecycle checks passed\n";
		return 0;
	}
	catch (const std::exception& error) {
		std::cerr << error.what() << '\n';
		return 1;
	}
}
