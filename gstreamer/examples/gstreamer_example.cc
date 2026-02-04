// gstreamer
#include "gstreamer.h"

// gemc
#include "glogger.h"
#include "gdynamicdigitization.h"
#include "gutilities.h"
#include "gthreads.h"

// c++
#include <atomic>         // std::atomic<T>: lock-free, thread-safe integers, flags…
#include <ranges>         // std::views::iota – range of integers 0,1,…,n-1
#include <vector>
#include <memory>         // smart pointers
#include <unordered_map>

/**
 * \file gstreamer_example.cc
 * \brief Multithreaded example showing how to publish events to gstreamer plugins.
 *
 * This example demonstrates a typical flow:
 * - Build a shared GOptions instance including gstreamer options via gstreamer::defineOptions().
 * - Initialize one gstreamer map per worker thread using gstreamer::gstreamersMapPtr().
 * - Open streamer connections inside the worker thread (one streamer instance per thread).
 * - Build synthetic event data (true + digitized hits) using gdynamicdigitization routines.
 * - Publish events via \ref GStreamer::publishEventData "publishEventData()".
 * - Close connections at the end (which also flushes any remaining buffered events).
 */

const std::string plugin_name = "test_gdynamic_plugin";

/**
 * \brief Run a synthetic event simulation in multiple worker threads and publish results via gstreamer.
 *
 * Threading model:
 * - A single atomic counter assigns distinct event numbers to workers.
 * - Each worker thread owns its own streamer map (no streamer sharing across threads).
 * - Each worker publishes events to all configured streamers, relying on the streamer buffering
 *   (configured via the \c ebuffer option).
 *
 * \param nevents Total number of events to produce across all threads.
 * \param nthreads Number of worker threads.
 * \param log Logger used for progress and diagnostics.
 * \param dynamicRoutinesMap Map of dynamic digitization routines keyed by plugin name.
 * \param gopts Options container used to configure streamers and other modules.
 */
void run_simulation_in_threads(int                                                              nevents,
                               int                                                              nthreads,
                               const std::shared_ptr<GLogger>&                                  log,
                               const std::shared_ptr<const gdynamicdigitization::dRoutinesMap>& dynamicRoutinesMap,
                               const std::shared_ptr<GOptions>&                                 gopts) {
	// Thread-safe integer counter starts at 1.
	// fetch_add returns the old value *and* bumps.
	// Zero contention: each thread fetches the next free event number.
	std::atomic<int> next{1};

	// Pool of jthreads. jthread joins in its destructor so we don’t need an
	// explicit loop at the end.
	// Each element represents one worker thread running your event-processing lambda.
	// std::vector<std::jthread> pool; use this when C++20 is widely available.
	std::vector<jthread_alias> pool; // was std::vector<std::jthread>

	pool.reserve(nthreads);

	for (int tid = 0; tid < nthreads; ++tid) {
		// The capture [&, tid] gives the thread references to variables like tid
		pool.emplace_back([&, tid] // capture tid *by value*
		{
			// Start thread with a lambda.
			log->info(0, "worker ", tid, " started");

			int localCount = 0; // events built by *this* worker
			thread_local std::vector<std::unique_ptr<GEventDataCollection>> localRunData;

			// Create one streamer map for this thread and open all output connections.
			auto gstreamer_map = gstreamer::gstreamersMapPtr(gopts, tid);
			for (auto& [name, gstreamer] : *gstreamer_map) {
				if (!gstreamer->openConnection()) {
					log->error(1, "Failed to open connection for GStreamer ", name, " in thread ", tid);
				}
			}

			while (true) {
				// Repeatedly asks the shared atomic counter for “the next unclaimed event
				// number,” processes that event, stores the result, and goes back for more.
				// memory_order_relaxed: we only need *atomicity*, no ordering.
				int evn = next.fetch_add(1, std::memory_order_relaxed);
				// atomically returns the current value and increments it by 1.
				if (evn > nevents) break; // exit the while loop

				// Create an event header and event container for this thread.
				auto gevent_header = GEventHeader::create(gopts, tid);
				auto eventData     = std::make_shared<GEventDataCollection>(gopts, std::move(gevent_header));

				// Create a small synthetic detector dataset:
				// each event has 10 hits, and each hit is converted into true and digitized data.
				for (unsigned i = 1; i < 11; i++) {
					auto hit       = GHit::create(gopts);
					auto true_data = dynamicRoutinesMap->at(plugin_name)->collectTrueInformation(hit, i);
					auto digi_data = dynamicRoutinesMap->at(plugin_name)->digitizeHit(hit, i);

					eventData->addDetectorDigitizedData("ctof", std::move(digi_data));
					eventData->addDetectorTrueInfoData("ctof", std::move(true_data));
				}

				log->info(0, "worker ", tid, " event ", evn, " has ",
				          eventData->getDataCollectionMap().at("ctof")->getDigitizedData().size(), " digitized hits");

				// Publish the event to each configured streamer.
				// The streamer may buffer and flush based on its configured ebuffer value.
				for (const auto& [name, gstreamer] : *gstreamer_map) {
					gstreamer->publishEventData(eventData);
				}

				++localCount; // tally for this worker
			}

			// Close streamer connections.
			// Close implies a flush of any remaining buffered events.
			for (const auto& [name, gstreamer] : *gstreamer_map) {
				if (!gstreamer->closeConnection()) {
					log->error(1, "Failed to close connection for GStreamer ", name, " in thread ", tid);
				}
			}

			log->info(0, "worker ", tid, " processed ", localCount, " events");
		}); // jthread constructor launches the thread immediately
	}       // pool’s destructor blocks until every jthread has joined
}


// emulation of a run of events, collecting and publish data in separate threads
int main(int argc, char* argv[]) {
	// Create GOptions using gstreamer::defineOptions, which aggregates options from gstreamer and gdynamicdigitization.
	auto gopts = std::make_shared<GOptions>(argc, argv, gstreamer::defineOptions());

	// Create a module logger for this example.
	auto log = std::make_shared<GLogger>(gopts, SFUNCTION_NAME, GSTREAMER_LOGGER);

	constexpr int nevents  = 200;
	constexpr int nthreads = 4;

	// Load dynamic digitization routines and their constants.
	auto dynamicRoutinesMap = gdynamicdigitization::dynamicRoutinesMap({plugin_name}, gopts);
	if (dynamicRoutinesMap->at(plugin_name)->loadConstants(1, "default") == false) {
		log->error(1, "Failed to load constants for dynamic routine", plugin_name,
		           "for run number 1 with variation 'default'.");
	}

	run_simulation_in_threads(nevents, nthreads, log, dynamicRoutinesMap, gopts);

	return EXIT_SUCCESS;
}
