/**
* \file event_example.cc
 * \brief Example demonstrating event data collection.
 *
 * \page gdata_event_example Event data collection example
 *
 * \section event_overview Overview
 * This example emulates an event loop where each event produces hit data for one detector.
 * Each event constructs a \ref GEventDataCollection, which owns per-hit objects:
 * - \ref GDigitizedData  digitized observables (ADC/TDC-like values, etc.)
 * - \ref GTrueInfoData   truth observables (energy deposition, positions, etc.)
 *
 * \section event_threading Threading model
 * The example uses a simple work-stealing pattern:
 * - A shared atomic counter hands out event numbers.
 * - Each worker thread builds independent \ref GEventDataCollection objects.
 * - Results are appended into a shared vector under a mutex.
 *
 * \section gdata_event_usage Usage
 * Build this example together with the GData library components and required GEMC utilities
 * (logging, options, touchable, threads abstraction).
 *
 * \author \n &copy; Maurizio Ungaro
 * \author e-mail: ungaro@jlab.org
 */

// gdata
#include "run/gRunDataCollection.h"

// gemc
#include "glogger.h"
#include "gthreads.h"

auto run_simulation_in_threads(int                              nevents,
                               int                              nthreads,
                               const std::shared_ptr<GOptions>& gopt,
                               const std::shared_ptr<GLogger>&  log)
	-> std::vector<std::shared_ptr<GEventDataCollection>> {
	std::mutex                                         collectorMtx;
	std::vector<std::shared_ptr<GEventDataCollection>> collected;

	// Thread-safe integer event counter starts at 1.
	std::atomic<int> next{1};

	// Pool of threads. (jthread_alias joins on destruction.)
	std::vector<jthread_alias> pool;
	pool.reserve(nthreads);

	for (int tid = 0; tid < nthreads; ++tid) {
		pool.emplace_back([&, tid] {
			log->info(0, "worker ", tid, " started");

			int localCount = 0;
			thread_local std::vector<std::shared_ptr<GEventDataCollection>> localRunData;

			while (true) {
				int evn = next.fetch_add(1, std::memory_order_relaxed);
				if (evn > nevents) { break; }

				// Create one event container (internally creates a header + one dummy hit for CTOF).
				auto event_data_collection = GEventDataCollection::create(gopt);
				localRunData.emplace_back(event_data_collection);

				++localCount;
			}

			{
				std::scoped_lock lk(collectorMtx);
				for (auto& evt : localRunData) { collected.emplace_back(evt); }
				localRunData.clear();
			}

			log->info(0, "worker ", tid, " processed ", localCount, " events");
		});
	}
	return collected;
}

int main(int argc, char* argv[]) {
	// Aggregate options for event-level data collection.
	auto gopts = std::make_shared<GOptions>(argc, argv, gevent_data::defineOptions());

	// Event-data logger.
	auto log = std::make_shared<GLogger>(gopts, SFUNCTION_NAME, GEVENTDATA_LOGGER);

	constexpr int nevents  = 10;
	constexpr int nthreads = 8;

	auto runData = run_simulation_in_threads(nevents, nthreads, gopts, log);

	// Demonstration: print the event numbers collected.
	for (size_t i = 0; i < runData.size(); i++) {
		log->info("event n. ", i + 1, " collected with local event number: ", runData[i]->getEventNumber());
	}

	return EXIT_SUCCESS;
}
