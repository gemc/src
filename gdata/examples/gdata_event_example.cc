/**
 * \file gdata_event_example.cc
 * \brief Example demonstrating event data collection in the GData library.
 *
 * \mainpage GData Event Data Example
 *
 * \section intro_sec Introduction
 * This example emulates a run of 10 events, where each event collects digitized hit data.
 * For each event:
 *  - A GEventDataCollectionHeader is created.
 *  - A GEventDataCollection is instantiated.
 *  - A single hit is created using a GTouchable and a default HitBitSet.
 *  - Digitized data is produced and added to the event's data collection.
 *
 * \section usage_sec Usage
 * Compile and run this example along with the GData library components (GEventDataCollection,
 * GTrueInfoData, GDigitizedData, etc.) and the associated logging and options modules.
 *
 * \n\n
 * \author \n &copy; Maurizio Ungaro
 * \author e-mail: ungaro@jlab.org
 * \n\n\n
 */

// gdata
#include "event/gEventDataCollection.h"
//#include "gdata_options.h"

// gemc
#include "glogger.h"
//#include <gtouchable_options.h>


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

auto run_simulation_in_threads(int                              nevents,
                               int                              nthreads,
                               const std::shared_ptr<GOptions>& gopt,
                               const std::shared_ptr<GLogger>&  log) -> std::vector<std::shared_ptr<GEventDataCollection>> {
	std::mutex                                         collectorMtx;
	std::vector<std::shared_ptr<GEventDataCollection>> collected;

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
		// The capture [&, tid] gives the thread references to variables like next, nevents, runDataMtx, etc.
		pool.emplace_back([&, tid] // capture tid *by value*
		{
			// start the thread with a lambda
			log->info(0, "worker ", tid, " started");

			int                                                             localCount = 0; // events built by *this* worker
			thread_local std::vector<std::shared_ptr<GEventDataCollection>> localRunData;

			while (true) {
				// repeatedly asks the shared atomic counter for “the next unclaimed event
				// number,” processes that event, stores the result, and goes back for more.
				// memory_order_relaxed: we only need *atomicity*, no ordering
				int evn = next.fetch_add(1, std::memory_order_relaxed); // atomically returns the current value and increments it by 1.
				if (evn > nevents) break;                               // exit the while loop

				auto event_data_collection = GEventDataCollection::create(gopt);
				localRunData.emplace_back(event_data_collection);

				++localCount; // tally for this worker
			}

			// braces to lock the mutex when it's constructed and unlocks when it is destroyed
			{
				std::scoped_lock lk(collectorMtx);
				for (auto& evt : localRunData) { collected.emplace_back(evt); }
				localRunData.clear();
			}

			log->info(0, "worker ", tid, " processed ", localCount, " events");
		}); // jthread constructor launches the thread immediately
	}       // pool’s destructor blocks until every jthread has joined
	return collected;
}


// emulation of a run of events, collecting data in separate threads

int main(int argc, char* argv[]) {
	// Create GOptions using gevent_data::defineOptions, which aggregates options from all gdata and gtouchable.
	auto gopts = std::make_shared<GOptions>(argc, argv, gevent_data::defineOptions());

	// Create loggers: one for gdata and one for gtouchable.
	auto log = std::make_shared<GLogger>(gopts, SFUNCTION_NAME, GEVENTDATA_LOGGER);

	constexpr int nevents  = 10;
	constexpr int nthreads = 8;

	auto runData = run_simulation_in_threads(nevents, nthreads, gopts, log);

	// For demonstration, we'll simply print the event numbers.
	for (size_t i = 0; i < runData.size(); i++) { log->info("event n. ", i + 1, " collected with local event number: ", runData[i]->getEventNumber()); }

	return EXIT_SUCCESS;
}
