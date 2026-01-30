/**
* \file run_example.cc
 * \brief Example demonstrating run-level integration of event data.
 *
 * \page gdata_run_example Run data integration example
 *
 * \section run_overview Overview
 * This example emulates integrating many events into a run summary.
 *
 * The workflow is:
 * 1) Build an event container (\ref GEventDataCollection) per event.
 * 2) Feed each event container into \ref GRunDataCollection::collect_event_data_collection().
 * 3) The run collection integrates per-detector data into a single entry per detector
 *    using accumulation semantics (summing observables across events).
 *
 * \section run_threading Threading model
 * The example shows a per-thread run collection (\c run_data_local) to accumulate data.
 * In production you typically either:
 * - keep a per-thread accumulator and merge at the end, or
 * - protect a shared accumulator with synchronization.
 *
 * \warning This file is an example; it focuses on showing the GData API usage rather than
 * providing a finalized reduction strategy.
 *
 * \section gdata_run_usage Usage
 * Build this example together with the GData library components and required GEMC utilities.
 *
 * \author \n &copy; Maurizio Ungaro
 * \author e-mail: ungaro@jlab.org
 */

// gdata
#include "run/gRunDataCollection.h"
#include "event/gEventDataCollection.h"

// gemc
#include <memory>

#include "glogger.h"
#include "gthreads.h"

auto run_simulation_in_threads(int                              nevents,
                               int                              nthreads,
                               const std::shared_ptr<GOptions>& gopt,
                               const std::shared_ptr<GLogger>&  log) -> std::shared_ptr<GRunDataCollection> {
	std::mutex collectorMtx;

	auto grun_header = std::make_unique<GRunHeader>(gopt, 1); // using run id 1, thread by default is -1
	auto run_data    = std::make_shared<GRunDataCollection>(gopt, std::move(grun_header));

	std::atomic<int> next{1};

	std::vector<jthread_alias> pool;
	pool.reserve(nthreads);

	for (int tid = 0; tid < nthreads; ++tid) {
		pool.emplace_back([&, tid] {
			log->info(0, "worker ", tid, " started");

			int localCount = 0;

			thread_local auto grun_header_local = std::make_unique<GRunHeader>(gopt, 1, tid);
			thread_local auto run_data_local    = std::make_shared<GRunDataCollection>(gopt, std::move(grun_header_local));

			while (true) {
				int evn = next.fetch_add(1, std::memory_order_relaxed);
				if (evn > nevents) break;

				auto event_data_collection = GEventDataCollection::create(gopt);
				run_data_local->collect_event_data_collection(event_data_collection);

				++localCount;
			}

			log->info(0, "worker ", tid, " processed ", localCount, " events");
		});
	}

	return run_data;
}

int main(int argc, char* argv[]) {
	// Aggregate options for run-level data collection.
	auto gopts = std::make_shared<GOptions>(argc, argv, grun_data::defineOptions());

	// Run-data logger.
	auto log = std::make_shared<GLogger>(gopts, SFUNCTION_NAME, GRUNDATA_LOGGER);

	constexpr int nevents  = 100;
	constexpr int nthreads = 1;

	auto runData = run_simulation_in_threads(nevents, nthreads, gopts, log);

	return EXIT_SUCCESS;
}
