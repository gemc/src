// gstreamer
#include "gstreamer.h"

// gemc
#include "glogger.h"
#include "gdynamicdigitization.h"
#include "gutilities.h"
#include "gdata_options.h"
#include "gtouchable_options.h"

// this example uses the gplugin built by gdynamicDigitization example
const std::string plugin_name = "test_gdynamic_plugin";

// c++
#include <atomic>         // std::atomic<T>: lock-free, thread-safe integers, flags…
//#include <jthread>      // C++20 std::jthread: joins automatically in its dtor (destructor)
#include <thread>         // replaces <jthread> until C++20 is widely available. remove this line when <jthread> is available.
#include <mutex>          // std::mutex, std::scoped_lock
#include <ranges>         // std::views::iota – range of integers 0,1,…,n-1
#include <vector>
#include <memory>         // smart pointers
#include <unordered_map>

// ===== portable jthread-like wrapper =========================================
// If real std::jthread is present, use it. Otherwise, define a minimal shim
// that joins in the destructor (no stop_token support, but good enough here).

#if defined(__cpp_lib_jthread)   // header exists
#include <jthread>
using jthread_alias = std::jthread;
#else
// join: pause right here until that thread is finished.
class jthread_alias : public std::thread {
public:
	using std::thread::thread; // inherit all ctors
	~jthread_alias() { if (joinable()) join(); }
	jthread_alias(jthread_alias&&) noexcept            = default;
	jthread_alias& operator=(jthread_alias&&) noexcept = default;
	// no copy
	jthread_alias(const jthread_alias&)            = delete;
	jthread_alias& operator=(const jthread_alias&) = delete;
};
#endif

// -----------------------------------------------------------------------------
// Build a single event and hand back exclusive ownership
// Trailing-return syntax: auto f(args) -> ReturnType
// -----------------------------------------------------------------------------
auto build_event(int                                                                           evn,
                 const std::shared_ptr<GLogger>&                                               loge,
                 const std::shared_ptr<GLogger>&                                               logt,
                 const std::unordered_map<std::string, std::shared_ptr<GDynamicDigitization>>& dynamicRoutinesMap) -> std::unique_ptr<GEventDataCollection> {

	// --------- Header construction ------------------------------------------
	// unique_ptr means *exactly one* owner.  We start as the owner and later
	auto gheader = std::make_unique<GEventDataCollectionHeader>(evn, evn % 3, loge);
	auto eventData = std::make_unique<GEventDataCollection>(std::move(gheader), loge);


	// --------- Rest of the event payload ------------------------------------
	const HitBitSet           hitBits("000001"); // test pattern
	const std::vector<double> dims = {1.0, 20.0, static_cast<double>(evn)};

	// ---- Lambda expression --------------------------------------------------
	// Syntax:   [capture clause] (params) -> ret { body }
	//
	// [&]   captures every variable used inside by *reference* (cheap, but be
	//       sure they outlive the lambda).  Here we need 'evn', 'dims', 'logt'.
	//
	// The lambda returns a unique_ptr<GTouchable>.  Because return type can be
	// deduced, we omit the trailing '-> std::unique_ptr<...>'.
	//
	// Lambdas are just syntactic sugar for generating a tiny, unnamed struct
	// with operator() – you can pass them around like function objects.
	auto make_touchable = [&](int factor) {
		return std::make_unique<GTouchable>(
		                                    "readout",
		                                    "sector: " + std::to_string(evn * factor) +
		                                    ", paddle: " + std::to_string(evn * factor),
		                                    dims,
		                                    logt);
	};

	// Build two touchables, two hits
	auto ctof1 = make_touchable(1);
	auto ctof2 = make_touchable(2);

	auto hit1 = std::make_unique<GHit>(ctof1.get(), hitBits);
	auto hit2 = std::make_unique<GHit>(ctof2.get(), hitBits);

	hit1->randomizeHitForTesting(10);
	hit2->randomizeHitForTesting(5);

	// not using dynamicRoutinesMap[key] because it modifies the map if the key is not found
	const auto& droutine  = dynamicRoutinesMap.at(plugin_name);
	auto        truedata1 = droutine->collectTrueInformation(hit1.get(), 1);
	auto        truedata2 = droutine->collectTrueInformation(hit2.get(), 2);

	auto digi1 = droutine->digitizeHit(hit1.get(), 1);
	auto digi2 = droutine->digitizeHit(hit2.get(), 2);

	eventData->addDetectorDigitizedData(plugin_name, digi1);
	eventData->addDetectorDigitizedData(plugin_name, digi2);

	// ctof*, hit* go out of scope → deleted automatically
	// unique_ptr supports *return value optimisation* – no explicit std::move
	return eventData;
}

// -----------------------------------------------------------------------------
// Multithreaded driver
// Each thread repeatedly grabs the next event number from an atomic counter
// -----------------------------------------------------------------------------
void run_simulation(int                                                                           nevents,
                    int                                                                           nthreads,
                    GOptions*                                                                     gopts,
                    const std::unordered_map<std::string, std::shared_ptr<GDynamicDigitization>>& dynamicRoutinesMap,
                    std::vector<std::unique_ptr<GEventDataCollection>>&                           runData,
                    std::mutex&                                                                   runDataMtx) {
	// thread-safe integer counter starts at 1.
	// fetch_add returns the old value *and* bumps.
	// Zero contention: each thread fetches the next free event number.
	std::atomic<int> next{1};

	// Pool of jthreads.  jthread joins in its destructor so we don’t need an
	// explicit loop at the end.
	// each element represents one worker thread running your event-processing lambda.
	// std::vector<std::jthread> pool; use this when C++20 is widely available
	std::vector<jthread_alias> pool; // was std::vector<std::jthread>

	pool.reserve(nthreads);


	// C++20 ranges: views::iota(0,n) generates 0,1,...,n-1 lazily.
	// for (int /*tid*/ : std::views::iota(0, nthreads))
	// not all compilers support ranges yet, so use a classic loop:
	// creates nthreads worker threads
	// The outer loop launches N independent worker threads; each worker loops,
	// atomically grabs the next event number to ensure no duplication,
	// builds that event, stores it under a mutex, and exits when every event
	// up to nevents has been processed—while the main thread simply
	// waits for all workers to join at the end.
	for (int tid = 0; tid < nthreads; ++tid) {
		// The capture [&, tid] gives the thread references to variables like next, nevents, runDataMtx, etc.
		pool.emplace_back([&, tid] // capture tid *by value*
		{                          // start thread with a lambda

			// one set of loggers per thread
			auto logs_t = std::make_shared<GLogger>(gopts, GSTREAMER_LOGGER, "gstreamer_example: worker " + std::to_string(tid));
			auto loge_t = std::make_shared<GLogger>(gopts, DATA_LOGGER, "gstreamer_example example: GEventDataCollection for worker " + std::to_string(tid));
			auto logt_t = std::make_shared<GLogger>(gopts, TOUCHABLE_LOGGER, "gstreamer_example example: GTouchable for worker " + std::to_string(tid));

			logs_t->info(0, "worker ", tid, " started");

			int localCount = 0; // events built by *this* worker

			thread_local const auto& streamers = gstreamer::gstreamersMap(gopts, tid);

			while (true) {
				// repeatedly asks the shared atomic counter for “the next unclaimed event
				// number,” processes that event, stores the result, and goes back for more.
				// memory_order_relaxed: we only need *atomicity*, no ordering
				int evn = next.fetch_add(1, std::memory_order_relaxed); // atomically returns the current value and increments it by 1.
				if (evn > nevents) break;                               // exit the while loop

				auto event = build_event(evn, loge_t, logt_t, dynamicRoutinesMap);

				for (const auto& [name, gstreamer] : streamers) {
					// publish the event to the gstreamer
					gstreamer->publishEventData(event.get());
				}

				// ---- Critical section: push into the shared vector ----------
				// std::scoped_lock locks *all* mutexes passed to it and unlocks
				// automatically when it goes out of scope (RAII).
				// {
				// 	// worker thread adds its freshly built event to the shared runData vector.
				// 	// scoped lock: locks the mutex in its constructor and automatically unlocks in
				// 	// its destructor (when the local variable lk goes out of scope).
				// 	std::scoped_lock lk(runDataMtx);
				// 	// std::vector is not thread-safe for concurrent writers, that's why we need the lock
				// 	runData.emplace_back(std::move(event));
				// }


				++localCount; // tally for this worker
			}

			// close the connection for this thread
			for (const auto& [name, gstreamer] : streamers) {
				// publish the event to the gstreamer
				gstreamer->closeConnection();
			}

			// ---------- per-thread summary log ----------
			// GLogger::info(level, ...args...)
			// Level 0: always shown unless you filter it out in your options.
			loge_t->info(0, "worker ", tid, " processed ", localCount, " events");
		}); // jthread constructor launches the thread immediately
	}       // pool’s destructor blocks until every jthread has joined
}


#include <TROOT.h>

// notice runData is not really used here, but we keep the code as reference on how to accumulate
// events in a thread-safe way into a shared vector.
int main(int argc, char* argv[]) {
	ROOT::EnableThreadSafety();

	// runData holds the finished events.  We store them as *unique_ptr* because
	// each event is owned by the container and *only* by the container (single
	// ownership → choose unique_ptr, not shared_ptr).
	std::vector<std::unique_ptr<GEventDataCollection>> runData;

	// Protect runData from concurrent push_back calls.  Only the push itself is
	// guarded; building events happens outside the critical section.
	std::mutex runDataMtx;

	// Create GOptions using gdata::defineOptions, which aggregates options from gdata and gtouchable.
	auto gopts = new GOptions(argc, argv, gstreamer::defineOptions());

	auto logs = std::make_shared<GLogger>(gopts, GSTREAMER_LOGGER, "gstreamer_example: main");

	// using the dynamic digitization plugin built by the gdynamicdigitization example
	std::vector<std::string> plugins = {plugin_name};

	const auto& dynRoutinesConstMap = gdynamicdigitization::dynamicRoutinesMap(plugins, gopts);
	if (dynRoutinesConstMap.at(plugin_name)->loadConstants(1, "default") == false) {
		logs->error(1, "Failed to load constants for dynamic routine", plugin_name, "for run number 1 with variation 'default'.");
	}

	constexpr int nevents  = 200;
	constexpr int nthreads = 8;

	run_simulation(nevents, nthreads, gopts,
	               dynRoutinesConstMap,
	               runData,
	               runDataMtx);

	logs->info(0, "Finished processing ", runData.size(), " events.");

	return EXIT_SUCCESS;
}
