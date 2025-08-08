/// \file plugin_load_example.cc

// example on how to use the gdynamic library

// gdynamic
#include "gdynamicdigitization.h"
#include "gdynamicdigitization_options.h"

// gemc
#include "gfactory.h"
#include "event/gEventDataCollection.h"

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

auto run_simulation_in_threads(int                                                              nevents,
                               int                                                              nthreads,
                               const std::shared_ptr<GOptions>&                                 gopt,
                               const std::shared_ptr<GLogger>&                                  log,
                               const std::shared_ptr<const gdynamicdigitization::dRoutinesMap>& dynamicRoutinesMap) -> std::vector<std::unique_ptr<GEventDataCollection>> {
	std::mutex                                         collectorMtx;
	std::vector<std::unique_ptr<GEventDataCollection>> collected;

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

			while (true) {
				// repeatedly asks the shared atomic counter for “the next unclaimed event
				// number,” processes that event, stores the result, and goes back for more.
				// memory_order_relaxed: we only need *atomicity*, no ordering
				int evn = next.fetch_add(1, std::memory_order_relaxed); // atomically returns the current value and increments it by 1.
				if (evn > nevents) break;                               // exit the while loop

				auto gheader   = GEventHeader::create(gopt);
				auto eventData = std::make_unique<GEventDataCollection>(gopt, std::move(gheader));

				// each event has 2 hits
				for (unsigned i = 1; i < 3; i++) {
					auto hit       = GHit::create(gopt);
					auto true_data = dynamicRoutinesMap->at(plugin_name)->collectTrueInformation(hit, i);
					auto digi_data = dynamicRoutinesMap->at(plugin_name)->digitizeHit(hit, i);

					eventData->addDetectorDigitizedData("ctof", std::move(digi_data));
					eventData->addDetectorTrueInfoData("ctof", std::move(true_data));
				}

				log->info(0, "worker ", tid, " event ", evn, " has ", eventData->getDataCollectionMap().at("ctof")->getDigitizedData().size(), " digitized hits");

				localRunData.emplace_back(std::move(eventData));

				++localCount; // tally for this worker
			}

			// braces to locks the mutex when it's constructed and unlocks when it is destroyed
			{
				std::scoped_lock lk(collectorMtx);
				for (auto& evt : localRunData) {
					// only collect 2 events total so that the log doesn't go crazy with the destructor
					if (collected.size() >= 2) break;
					collected.emplace_back(std::move(evt));
				}
				localRunData.clear();
			}

			log->info(0, "worker ", tid, " processed ", localCount, " events");
		}); // jthread constructor launches the thread immediately
	}       // pool’s destructor blocks until every jthread has joined
	return collected;
}


// emulation of a run of events, collecting data in separate threads


int main(int argc, char* argv[]) {
	// Create GOptions using gdata::defineOptions, which aggregates options from gdata and gtouchable.
	auto gopts = std::make_shared<GOptions>(argc, argv, gdynamicdigitization::defineOptions());

	// duplicate plugin logger here
	auto log = std::make_shared<GLogger>(gopts, SFUNCTION_NAME, PLUGIN_LOGGER);

	constexpr int nevents  = 10;
	constexpr int nthreads = 8;

	auto dynamicRoutinesMap = gdynamicdigitization::dynamicRoutinesMap({plugin_name}, gopts);
	if (dynamicRoutinesMap->at(plugin_name)->loadConstants(1, "default") == false) {
		log->error(1, "Failed to load constants for dynamic routine", plugin_name, "for run number 1 with variation 'default'.");
	}

	auto runData = run_simulation_in_threads(nevents, nthreads, gopts, log, dynamicRoutinesMap);

	// For demonstration, we'll simply print the event numbers.
	for (size_t i = 0; i < runData.size(); i++) { log->info(" > Event ", i + 1, " collected with local event number: ", runData[i]->getEventNumber()); }

	return EXIT_SUCCESS;
}
