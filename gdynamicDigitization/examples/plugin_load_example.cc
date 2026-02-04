/// \file plugin_load_example.cc
///
/// \brief Example demonstrating how to load and use a gdynamic digitization plugin.
///
/// This example shows:
/// - constructing a shared GOptions instance using gdynamicdigitization::defineOptions()
/// - loading a dynamic routine map with gdynamicdigitization::dynamicRoutinesMap()
/// - calling \ref GDynamicDigitization::loadConstants "loadConstants()"
/// - running a small multi-threaded "event build" loop
/// - collecting a limited number of events to keep output manageable
///
/// \note
/// This is an example program, so it intentionally favors clarity over performance tuning.

// gdynamic
#include "gdynamicdigitization.h"
#include "gdynamicdigitization_options.h"

// gemc
#include "gfactory.h"
#include "event/gEventDataCollection.h"
#include "gthreads.h"

const std::string plugin_name = "test_gdynamic_plugin";

/**
 * \brief Runs a small simulated event loop using multiple worker threads.
 *
 * Each worker thread repeatedly claims the next event number from an atomic counter,
 * constructs a fresh event container, creates a few hits, and processes them through
 * the loaded dynamic routine.
 *
 * The routine collects at most two events into the shared output vector to keep the
 * example output small (this also reduces destructor chatter in logs).
 *
 * Threading model:
 * - Uses an atomic counter (\c next) to distribute unique event numbers
 * - Uses a mutex to protect the shared output vector
 * - Uses per-thread storage (\c thread_local) to accumulate events before acquiring the lock
 *
 * \param nevents Number of events to simulate (upper bound; some may not be collected).
 * \param nthreads Number of worker threads to launch.
 * \param gopt Shared options.
 * \param log Logger used for informational output.
 * \param dynamicRoutinesMap Map of loaded dynamic routines.
 * \return A vector of collected events (limited to two in this example).
 */
auto run_simulation_in_threads(int                                                              nevents,
                               int                                                              nthreads,
                               const std::shared_ptr<GOptions>&                                 gopt,
                               const std::shared_ptr<GLogger>&                                  log,
                               const std::shared_ptr<const gdynamicdigitization::dRoutinesMap>& dynamicRoutinesMap)
	-> std::vector<std::unique_ptr<GEventDataCollection>> {
	std::mutex                                         collectorMtx;
	std::vector<std::unique_ptr<GEventDataCollection>> collected;

	// Thread-safe event counter starts at 1; fetch_add returns the old value and increments.
	std::atomic<int> next{1};

	// Pool of threads. jthread_alias joins in its destructor.
	std::vector<jthread_alias> pool;
	pool.reserve(nthreads);

	for (int tid = 0; tid < nthreads; ++tid) {
		pool.emplace_back([&, tid] {
			log->info(0, "worker ", tid, " started");

			int localCount = 0; // events processed by this worker
			thread_local std::vector<std::unique_ptr<GEventDataCollection>> localRunData;

			while (true) {
				int evn = next.fetch_add(1, std::memory_order_relaxed);
				if (evn > nevents) break;

				auto gevent_header = GEventHeader::create(gopt);
				auto eventData     = std::make_unique<GEventDataCollection>(gopt, std::move(gevent_header));

				// Each event has 2 hits in this example.
				for (unsigned i = 1; i < 3; i++) {
					auto hit       = GHit::create(gopt);
					auto true_data = dynamicRoutinesMap->at(plugin_name)->collectTrueInformation(hit, i);
					auto digi_data = dynamicRoutinesMap->at(plugin_name)->digitizeHit(hit, i);

					eventData->addDetectorDigitizedData("ctof", std::move(digi_data));
					eventData->addDetectorTrueInfoData("ctof", std::move(true_data));
				}

				log->info(0, "worker ", tid, " event ", evn, " has ",
				          eventData->getDataCollectionMap().at("ctof")->getDigitizedData().size(), " digitized hits");

				localRunData.emplace_back(std::move(eventData));
				++localCount;
			}

			// Lock only while moving selected events into the shared output container.
			{
				std::scoped_lock lk(collectorMtx);
				for (auto& evt : localRunData) {
					if (collected.size() >= 2) break;
					collected.emplace_back(std::move(evt));
				}
				localRunData.clear();
			}

			log->info(0, "worker ", tid, " processed ", localCount, " events");
		});
	}

	// pool destructor joins all threads.
	return collected;
}

/**
 * \brief Example program entry point.
 *
 * Steps:
 * 1. Create options using gdynamicdigitization::defineOptions().
 * 2. Create a logger for this example process.
 * 3. Load the dynamic routine map.
 * 4. Load constants for the selected plugin.
 * 5. Run a short multi-threaded simulation loop.
 *
 * \param argc Standard argc.
 * \param argv Standard argv.
 * \return EXIT_SUCCESS on success.
 */
int main(int argc, char* argv[]) {
	// Create GOptions using gdynamicdigitization::defineOptions(), which aggregates options
	// from this module and its dependencies.
	auto gopts = std::make_shared<GOptions>(argc, argv, gdynamicdigitization::defineOptions());

	// Example-level logger.
	auto log = std::make_shared<GLogger>(gopts, SFUNCTION_NAME, PLUGIN_LOGGER);

	constexpr int nevents  = 10;
	constexpr int nthreads = 8;

	auto dynamicRoutinesMap = gdynamicdigitization::dynamicRoutinesMap({plugin_name}, gopts);

	if (dynamicRoutinesMap->at(plugin_name)->loadConstants(1, "default") == false) {
		log->error(1, "Failed to load constants for dynamic routine ", plugin_name,
		           " for run number 1 with variation 'default'.");
	}

	auto runData = run_simulation_in_threads(nevents, nthreads, gopts, log, dynamicRoutinesMap);

	// Print the collected events (not all processed events are collected in this example).
	for (size_t i = 0; i < runData.size(); i++) {
		log->info(" > Event ", i + 1, " collected with local event number: ", runData[i]->getEventNumber());
	}

	return EXIT_SUCCESS;
}
