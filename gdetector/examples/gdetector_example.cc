// gdetector
#include "gdetectorConstruction.h"
#include "gdetector_options.h"

// gemc
#include "glogger.h"
#include "event/gEventDataCollection.h"
#include "gdynamicdigitizationConventions.h"
#include "gthreads.h"

// geant4
#include "G4RunManagerFactory.hh"
#include "QBBC.hh"

// c++
#include <atomic>         // std::atomic<T>: lock-free, thread-safe integers, flags…
#include <vector>
#include <memory>         // smart pointers


auto run_simulation_in_threads(int                                                 nevents,
                               int                                                 nthreads,
                               const std::shared_ptr<GOptions>&                    gopts,
                               const std::shared_ptr<GLogger>&                     log,
                               const std::shared_ptr<const GDetectorConstruction>& gdetector) -> std::vector<std::shared_ptr<GEventDataCollection>> {
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
		// The capture [&, tid] gives the thread references to variables like tid
		pool.emplace_back([&, tid] // capture tid *by value*
		{
			log->info(0, "worker ", tid, " started");

			int localCount = 0; // events built by *this* worker

			thread_local std::vector<std::shared_ptr<GEventDataCollection>> localRunData;

			while (true) {
				// repeatedly asks the shared atomic counter for “the next unclaimed event
				// number,” processes that event, stores the result, and goes back for more.
				// memory_order_relaxed: we only need *atomicity*, no ordering
				int evn = next.fetch_add(1, std::memory_order_relaxed); // atomically returns the current value and increments it by 1.
				if (evn > nevents) break;                               // exit the while loop

				// flux does not need variation or run number
				std::string sdname    = "flux";
				int         runNumber = 1;
				std::string variation = "default";

				auto gevent_header      = GEventHeader::create(gopts, tid);
				auto eventData    = std::make_shared<GEventDataCollection>(gopts, std::move(gevent_header));
				auto digi_routine = gdetector->get_digitization_routines_for_sdname(sdname);
				log->debug(NORMAL, "Calling ", sdname, " loadConstants for run ", runNumber);

				if (digi_routine->loadConstants(runNumber, variation) == false) {
					log->error(ERR_LOADCONSTANTFAIL, "Failed to load constants for ", sdname, " for run ", runNumber, " with variation ", variation);
				}

				log->debug(NORMAL, "Calling ", sdname, " loadTT for run ", runNumber);
				if (digi_routine->loadTT(runNumber, variation) == false) {
					log->error(ERR_LOADTTFAIL, "Failed to load translation table for ", sdname, " for run ", runNumber, " with variation ", variation);
				}
				// each event has 10 hits
				for (unsigned i = 1; i < 11; i++) {
					auto hit       = GHit::create(gopts);
					auto true_data = digi_routine->collectTrueInformation(hit, i);
					auto digi_data = digi_routine->digitizeHit(hit, i);

					eventData->addDetectorDigitizedData("flux", std::move(digi_data));
					eventData->addDetectorTrueInfoData("flux", std::move(true_data));
				}

				const auto& flux_data_it = eventData->getDataCollectionMap().find("flux");


				if (flux_data_it != eventData->getDataCollectionMap().end()) {
					const auto& digitized_data = flux_data_it->second->getDigitizedData();
					log->info(0, "worker ", tid, " event ", evn, " has ", digitized_data.size(), " digitized hits");
				}

				++localCount; // tally for this worker
			}

			// braces to locks the mutex when it's constructed and unlocks when it is destroyed
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


// emulation of a run of events, collecting and publish data in separate threads
int main(int argc, char* argv[]) {
	// Create GOptions using gdata::defineOptions, which aggregates options from gdata and gtouchable.
	auto gopts = std::make_shared<GOptions>(argc, argv, gdetector::defineOptions());

	// Create loggers: one for gdata and one for gtouchable.
	auto log = std::make_shared<GLogger>(gopts, SFUNCTION_NAME, GDETECTOR_LOGGER);

	constexpr int nevents  = 20;
	constexpr int nthreads = 2;

	auto runManager  = G4RunManagerFactory::CreateRunManager(G4RunManagerType::Default);
	auto physicsList = new QBBC;

	runManager->SetUserInitialization(physicsList);

	auto gdetector = std::make_shared<GDetectorConstruction>(gopts);
	auto gsystems = gsystem::getSystems(gopts);
	gdetector->reload_geometry(gsystems);

	auto runDat = run_simulation_in_threads(nevents, nthreads, gopts, log, gdetector);

	return EXIT_SUCCESS;
}
