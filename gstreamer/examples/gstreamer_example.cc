// gstreamer
#include "gstreamer.h"

// gemc
#include "glogger.h"
#include "gdynamicdigitization.h"
#include "gutilities.h"
#include "gdata_options.h"

// c++
#include <atomic>         // std::atomic<T>: lock-free, thread-safe integers, flags…
//#include <jthread>      // C++20 std::jthread: joins automatically in its dtor (destructor)
#include <thread>         // replaces <jthread> until C++20 is widely available. remove this line when <jthread> is available.
#include <ranges>         // std::views::iota – range of integers 0,1,…,n-1
#include <vector>
#include <memory>         // smart pointers
#include <unordered_map>

// TODO: remove when C++20 is widely available
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

const std::string plugin_name = "test_gdynamic_plugin";

void run_simulation_in_threads(int                                                              nevents,
                               int                                                              nthreads,
                               const std::shared_ptr<GLogger>&                                  log,
                               const std::shared_ptr<const gdynamicdigitization::dRoutinesMap>& dynamicRoutinesMap,
                               const std::shared_ptr<GOptions>&                                 gopts) {
	// thread-safe integer counter starts at 1.
	// fetch_add returns the old value *and* bumps.
	// zero contention: each thread fetches the next free event number.
	std::atomic<int> next{1};

	// pool of jthreads.  jthread joins in its destructor so we don’t need an
	// explicit loop at the end.
	// each element represents one worker thread running your event-processing lambda.
	// std::vector<std::jthread> pool; use this when C++20 is widely available
	std::vector<jthread_alias> pool; // was std::vector<std::jthread>

	pool.reserve(nthreads);

	for (int tid = 0; tid < nthreads; ++tid) {
		// The capture [&, tid] gives the thread references to variables like tid
		pool.emplace_back([&, tid] // capture tid *by value*
		{
			// start thread with a lambda
			log->info(0, "worker ", tid, " started");

			int                                                             localCount = 0; // events built by *this* worker
			thread_local std::vector<std::unique_ptr<GEventDataCollection>> localRunData;

			auto gstreamerMapPtr = gstreamer::gstreamersMapPtr(gopts, tid);

			while (true) {
				// repeatedly asks the shared atomic counter for “the next unclaimed event
				// number,” processes that event, stores the result, and goes back for more.
				// memory_order_relaxed: we only need *atomicity*, no ordering
				int evn = next.fetch_add(1, std::memory_order_relaxed); // atomically returns the current value and increments it by 1.
				if (evn > nevents) break;                               // exit the while loop

				auto gheader   = GEventHeader::create(gopts, tid);
				auto eventData = std::make_shared<GEventDataCollection>(gopts, std::move(gheader));

				// each event has 10 hits
				for (unsigned i = 1; i < 11; i++) {
					auto hit       = GHit::create(gopts);
					auto true_data = dynamicRoutinesMap->at(plugin_name)->collectTrueInformation(hit, i);
					auto digi_data = dynamicRoutinesMap->at(plugin_name)->digitizeHit(hit, i);

					eventData->addDetectorDigitizedData("ctof", std::move(digi_data));
					eventData->addDetectorTrueInfoData("ctof", std::move(true_data));
				}

				log->info(0, "worker ", tid, " event ", evn, " has ", eventData->getDataCollectionMap().at("ctof")->getDigitizedData().size(), " digitized hits");

				for (const auto& [name, gstreamer] : *gstreamerMapPtr) {
					// publish the event to the gstreamer
					gstreamer->publishEventData(eventData);
				}

				++localCount; // tally for this worker
			}

			// close streamers connections
			for (const auto& [name, gstreamer] : *gstreamerMapPtr) {
				if (!gstreamer->closeConnection()) { log->error(1, "Failed to close connection for GStreamer ", name, " in thread ", tid); }
			}

			log->info(0, "worker ", tid, " processed ", localCount, " events");
		}); // jthread constructor launches the thread immediately
	}       // pool’s destructor blocks until every jthread has joined
}

//#include <TROOT.h>

// emulation of a run of events, collecting and publish data in separate threads
int main(int argc, char* argv[]) {
//	ROOT::EnableThreadSafety();

	// Create GOptions using gdata::defineOptions, which aggregates options from gdata and gtouchable.
	auto gopts = std::make_shared<GOptions>(argc, argv, gstreamer::defineOptions());

	// Create loggers: one for gdata and one for gtouchable.
	auto log = std::make_shared<GLogger>(gopts, DATA_LOGGER, "gstreamer_example: main");

	constexpr int nevents  = 200;
	constexpr int nthreads = 4;

	auto dynamicRoutinesMap = gdynamicdigitization::dynamicRoutinesMap({plugin_name}, gopts);
	if (dynamicRoutinesMap->at(plugin_name)->loadConstants(1, "default") == false) {
		log->error(1, "Failed to load constants for dynamic routine", plugin_name, "for run number 1 with variation 'default'.");
	}

	run_simulation_in_threads(nevents, nthreads, log, dynamicRoutinesMap, gopts);

	return EXIT_SUCCESS;
}
