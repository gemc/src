// gstreamer
#include "gstreamer.h"

// gemc
#include "glogger.h"
#include "gdynamicdigitization.h"
#include "gutilities.h"
#include "gthreads.h"

// c++
#include <atomic>
#include <ranges>
#include <vector>
#include <memory>
#include <unordered_map>

/**
 * \file gstreamer_example.cc
 * \ingroup gstreamer_examples_api
 * \anchor gstreamer_example
 * \brief Multithreaded example showing how to publish synthetic event data through the gstreamer module.
 *
 * Summary:
 * This example creates one streamer map per worker thread, builds synthetic detector hits,
 * digitizes them, publishes them through all configured streamers, and closes all outputs at
 * the end of execution.
 *
 * Related module documentation:
 * - \ref gstreamer_module "gstreamer module"
 *
 * Typical command-line usage:
 * \code
 * ./gstreamer_example \
 *   -gstreamer="[{format: ascii, filename: out_ascii}, {format: json, filename: out_json}]" \
 *   -ebuffer=20
 * \endcode
 */

const std::string plugin_name = "test_gdynamic_plugin";

/**
 * \brief Run a synthetic multithreaded event loop and publish the results through configured streamers.
 *
 * This function demonstrates the intended ownership and threading model of the gstreamer module:
 * - one shared configuration object is used by all workers
 * - one atomic counter distributes event numbers across threads
 * - each worker creates its own streamer map through
 *   \ref gstreamer::gstreamersMapPtr "gstreamersMapPtr()"
 * - each streamer instance is used only by the thread that created it
 *
 * Event construction flow:
 * - create an event header for the current thread
 * - create one event collection
 * - create a small synthetic set of hits for detector \c "ctof"
 * - derive both true-information and digitized representations from the dynamic routine
 * - publish the event to every configured streamer
 *
 * The example intentionally uses a compact synthetic dataset so the control flow remains easy to
 * understand while still exercising the full event publication sequence.
 *
 * \param nevents Total number of events to generate across all workers.
 * \param nthreads Number of worker threads to launch.
 * \param log Shared logger used for progress and diagnostic messages.
 * \param dynamicRoutinesMap Dynamic digitization routines keyed by plugin name.
 * \param gopts Parsed options container used to configure the module and dependent components.
 */
void run_simulation_in_threads(int                                                              nevents,
                               int                                                              nthreads,
                               const std::shared_ptr<GLogger>&                                  log,
                               const std::shared_ptr<const gdynamicdigitization::dRoutinesMap>& dynamicRoutinesMap,
                               const std::shared_ptr<GOptions>&                                 gopts) {
	// Shared atomic event counter used to distribute work without external locking.
	std::atomic<int> next{1};

	// Thread pool. Each worker owns its local streamer map and processes an arbitrary number of events.
	std::vector<jthread_alias> pool;
	pool.reserve(nthreads);

	for (int tid = 0; tid < nthreads; ++tid) {
		pool.emplace_back([&, tid]
		{
			log->info(0, "worker ", tid, " started");

			int localCount = 0;

			// Create all configured streamers for this worker thread.
			auto gstreamer_map = gstreamer::gstreamersMapPtr(gopts, tid);

			// Open every backend connection before entering the event loop.
			for (auto& [name, gstreamer] : *gstreamer_map) {
				if (!gstreamer->openConnection()) {
					log->error(1, "Failed to open connection for GStreamer ", name, " in thread ", tid);
				}
			}

			while (true) {
				// Claim the next event number atomically.
				int evn = next.fetch_add(1, std::memory_order_relaxed);
				if (evn > nevents) break;

				// Build one event collection with a thread-specific header.
				auto gevent_header = GEventHeader::create(gopts, tid);
				auto eventData     = std::make_shared<GEventDataCollection>(gopts, std::move(gevent_header));

				// Populate a small synthetic detector dataset.
				for (unsigned i = 1; i < 11; i++) {
					auto hit       = GHit::create(gopts);
					auto true_data = dynamicRoutinesMap->at(plugin_name)->collectTrueInformation(hit, i);
					auto digi_data = dynamicRoutinesMap->at(plugin_name)->digitizeHit(hit, i);

					eventData->addDetectorDigitizedData("ctof", std::move(digi_data));
					eventData->addDetectorTrueInfoData("ctof", std::move(true_data));
				}

				log->info(0, "worker ", tid, " event ", evn, " has ",
				          eventData->getDataCollectionMap().at("ctof")->getDigitizedData().size(),
				          " digitized hits");

				// Publish the event to each configured streamer instance.
				for (const auto& [name, gstreamer] : *gstreamer_map) {
					gstreamer->publishEventData(eventData);
				}

				++localCount;
			}

			// Close all output backends. This also flushes any buffered events.
			for (const auto& [name, gstreamer] : *gstreamer_map) {
				if (!gstreamer->closeConnection()) {
					log->error(1, "Failed to close connection for GStreamer ", name, " in thread ", tid);
				}
			}

			log->info(0, "worker ", tid, " processed ", localCount, " events");
		});
	}
}

/**
 * \brief Entry point for the multithreaded gstreamer example.
 *
 * Responsibilities:
 * - define and parse module options through gstreamer::defineOptions()
 * - create a logger for the example
 * - load the test dynamic digitization routine
 * - run the synthetic multithreaded event loop
 *
 * \param argc Number of command-line arguments.
 * \param argv Command-line argument vector.
 * \return \c EXIT_SUCCESS on normal completion.
 */
int main(int argc, char* argv[]) {
	// Build the module option set and parse the command line.
	auto gopts = std::make_shared<GOptions>(argc, argv, gstreamer::defineOptions());

	// Create a logger scoped to this example.
	auto log = std::make_shared<GLogger>(gopts, SFUNCTION_NAME, GSTREAMER_LOGGER);

	constexpr int nevents  = 200;
	constexpr int nthreads = 4;

	// Load the dynamic digitization plugin used to generate synthetic detector content.
	auto dynamicRoutinesMap = gdynamicdigitization::dynamicRoutinesMap({plugin_name}, gopts);
	if (dynamicRoutinesMap->at(plugin_name)->loadConstants(1, "default") == false) {
		log->error(1, "Failed to load constants for dynamic routine", plugin_name,
		           "for run number 1 with variation 'default'.");
	}

	run_simulation_in_threads(nevents, nthreads, log, dynamicRoutinesMap, gopts);

	return EXIT_SUCCESS;
}