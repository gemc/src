/**
 * \file event_example.cc
 * \brief Event-level example demonstrating how to build and inspect \ref GEventDataCollection objects.
 *
 * \page gdata_event_example Event data collection example
 *
 * \section event_overview Overview
 * This example emulates a simplified event loop where each event produces hit data for one or more
 * sensitive detectors and stores them into a \ref GEventDataCollection.
 *
 * The \ref GEventDataCollection owns *per-hit* objects (event-level semantics):
 * - \ref GTrueInfoData:   "truth" observables derived from simulation / tracking (energy deposition, positions, times, …)
 * - \ref GDigitizedData:  "digitized" observables produced by electronics/digitization (ADC/TDC-like quantities, readout coords, …)
 *
 * For each event, data are organized as:
 * \code
 *   sdName -> GDataCollection
 *              - vector<unique_ptr<GTrueInfoData>>   (one per hit)
 *              - vector<unique_ptr<GDigitizedData>>  (one per hit)
 * \endcode
 *
 * \section event_demo What this example demonstrates
 * - Creating event containers (\ref GEventDataCollection::create()).
 * - Adding *additional hits* and *additional detectors* to the same event via:
 *   - \ref GEventDataCollection::addDetectorTrueInfoData()
 *   - \ref GEventDataCollection::addDetectorDigitizedData()
 * - Inspecting the stored data:
 *   - per-detector hit counts
 *   - identity strings (\ref GTrueInfoData::getIdentityString, \ref GDigitizedData::getIdentityString)
 *   - truth and digitized observable maps
 * - Demonstrating digitized filtering of streaming-readout (SRO) keys:
 *   - \ref GDigitizedData::getIntObservablesMap(int)
 *   - \ref GDigitizedData::getDblObservablesMap(int)
 *
 * \section event_threading Threading model
 * The example uses a simple work-distribution pattern:
 * - A shared atomic counter assigns event numbers.
 * - Each worker thread builds independent \ref GEventDataCollection objects.
 * - Results are appended to a shared vector under a mutex.
 *
 * \note \ref GEventHeader::create() and the test factories \ref GTrueInfoData::create and
 *       \ref GDigitizedData::create use internal thread-safe counters, so concurrent execution
 *       is supported for this example-style workload.
 *
 * \section gdata_event_usage Usage
 * Build this example together with the GData library components and required GEMC utilities
 * (logging, options, threads abstraction).
 *
 * \author \n &copy; Maurizio Ungaro
 * \author e-mail: ungaro@jlab.org
 */

// gdata
#include "event/gEventDataCollection.h" // explicit include for clarity in the example

// gemc
#include "glogger.h"
#include "gthreads.h"

// C++
#include <atomic>
#include <cstdlib>
#include <map>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

/**
 * \brief Convert a scalar map into a compact, deterministic string for logging.
 *
 * \details
 * This helper exists purely for examples/tests: it prints key/value pairs in the
 * order given by the container (std::map order is lexicographic by key).
 *
 * \tparam MapType A map-like type with string keys and printable scalar values.
 * \param m The map to stringify.
 * \return A single-line representation like "{k1=v1, k2=v2}".
 */
template <typename MapType>
static std::string map_to_string(const MapType& m) {
	std::ostringstream os;
	os << "{";
	bool first = true;
	for (const auto& [k, v] : m) {
		if (!first) os << ", ";
		first = false;
		os << k << "=" << v;
	}
	os << "}";
	return os.str();
}

/**
 * \brief Log a human-readable summary of all detector data contained in one event.
 *
 * \details
 * This performs a deep inspection of the \ref GEventDataCollection structure:
 * - loops over detectors (sdName)
 * - prints number of truth hits and digitized hits
 * - for each hit prints identity and observables
 *
 * For digitized data, this also prints both filtered views:
 * - which=0: non-SRO (physics/content) keys
 * - which=1: SRO-only keys (crate/slot/channel/timeAtElectronics/chargeAtElectronics)
 *
 * \param edc The event to inspect.
 * \param log Logger to emit messages to.
 */
static void dump_event(const std::shared_ptr<GEventDataCollection>& edc, const std::shared_ptr<GLogger>& log) {
	log->info(0, "------------------------------------------------------------");
	log->info(0, "Dumping event: local event number = ", edc->getEventNumber());
	log->info(0, "------------------------------------------------------------");

	const auto& dcm = edc->getDataCollectionMap();
	if (dcm.empty()) {
		log->info(0, "Event contains no detector data.");
		return;
	}

	for (const auto& [sdName, det] : dcm) {
		if (!det) {
			log->info(0, "Detector <", sdName, "> has a null GDataCollection pointer (unexpected).");
			continue;
		}

		const auto& truthHits = det->getTrueInfoData();
		const auto& digiHits  = det->getDigitizedData();

		log->info(0, "Detector <", sdName, ">: truthHits=", truthHits.size(), " digitizedHits=", digiHits.size());

		// ---- Truth hits
		for (size_t i = 0; i < truthHits.size(); ++i) {
			const auto& th = truthHits[i];
			if (!th) continue;

			const auto doubles = th->getDoubleVariablesMap();
			const auto strings = th->getStringVariablesMap();

			log->info(0, "  [truth hit ", i, "] id={", th->getIdentityString(), "}");
			log->info(0, "    doubles: ", map_to_string(doubles));

			// Strings are often empty in this toy factory, but the code shows how to inspect them.
			if (!strings.empty()) {
				log->info(0, "    strings: ", map_to_string(strings));
			}
			else {
				log->info(0, "    strings: {} (none)");
			}
		}

		// ---- Digitized hits
		for (size_t i = 0; i < digiHits.size(); ++i) {
			const auto& dh = digiHits[i];
			if (!dh) continue;

			const auto ints_non_sro  = dh->getIntObservablesMap(0);
			const auto ints_sro      = dh->getIntObservablesMap(1);
			const auto dbls_non_sro  = dh->getDblObservablesMap(0);
			const auto dbls_sro_only = dh->getDblObservablesMap(1);

			log->info(0, "  [digi  hit ", i, "] id={", dh->getIdentityString(), "}");
			log->info(0, "    int  non-SRO: ", map_to_string(ints_non_sro));
			log->info(0, "    int  SRO:     ", map_to_string(ints_sro));
			log->info(0, "    dbl  non-SRO: ", map_to_string(dbls_non_sro));
			log->info(0, "    dbl  SRO:     ", map_to_string(dbls_sro_only));

			// Convenience accessor demo (shows sentinel if missing):
			log->info(0, "    timeAtElectronics() = ", dh->getTimeAtElectronics());
		}
	}
}

/**
 * \brief Lightweight invariant checks for this example.
 *
 * \details
 * This performs a few sanity checks that are useful when evolving the library:
 * - event has at least one detector
 * - each detector collection has consistent hit vectors (non-null pointers)
 *
 * \note This is intentionally non-fatal; it logs diagnostics rather than aborting.
 *
 * \param edc The event to validate.
 * \param log Logger instance.
 */
static void validate_event_structure(const std::shared_ptr<GEventDataCollection>& edc, const std::shared_ptr<GLogger>& log) {
	const auto& dcm = edc->getDataCollectionMap();
	if (dcm.empty()) {
		log->info(0, "VALIDATION: event ", edc->getEventNumber(), " has no detectors (unexpected in this example).");
		return;
	}

	for (const auto& [sdName, det] : dcm) {
		if (!det) {
			log->info(0, "VALIDATION: detector <", sdName, "> has null GDataCollection pointer.");
			continue;
		}

		const auto& truthHits = det->getTrueInfoData();
		const auto& digiHits  = det->getDigitizedData();

		for (size_t i = 0; i < truthHits.size(); ++i) {
			if (!truthHits[i]) log->info(0, "VALIDATION: detector <", sdName, "> truth hit ", i, " is null.");
		}
		for (size_t i = 0; i < digiHits.size(); ++i) {
			if (!digiHits[i]) log->info(0, "VALIDATION: detector <", sdName, "> digitized hit ", i, " is null.");
		}

		// A very common expectation in practice: same number of truth and digitized hits per detector.
		// This is not guaranteed by the API (you can add one without the other), but it is a useful
		// check for this demo-style data producer.
		if (truthHits.size() != digiHits.size()) {
			log->info(0, "VALIDATION: detector <", sdName, "> truthHits(", truthHits.size(),
			          ") != digitizedHits(", digiHits.size(), ") in event ", edc->getEventNumber());
		}
	}
}

/**
 * \brief Produce a set of events using multiple worker threads.
 *
 * \details
 * Each event is created via \ref GEventDataCollection::create(), then extended to demonstrate:
 * - adding additional hits under the same detector key
 * - adding a second detector key
 *
 * \param nevents  Total number of events to generate.
 * \param nthreads Number of worker threads.
 * \param gopt     Shared options.
 * \param log      Logger.
 * \return Vector of event containers (one per event).
 */
static auto run_simulation_in_threads(int                              nevents,
                                      int                              nthreads,
                                      const std::shared_ptr<GOptions>& gopt,
                                      const std::shared_ptr<GLogger>&  log)
	-> std::vector<std::shared_ptr<GEventDataCollection>> {
	std::mutex                                         collectorMtx;
	std::vector<std::shared_ptr<GEventDataCollection>> collected;
	collected.reserve(static_cast<size_t>(nevents));

	// Thread-safe integer event counter starts at 1.
	std::atomic<int> next{1};

	// Pool of threads. (jthread_alias joins on destruction.)
	std::vector<jthread_alias> pool;
	pool.reserve(nthreads);

	for (int tid = 0; tid < nthreads; ++tid) {
		pool.emplace_back([&, tid] {
			log->info(0, "worker ", tid, " started");

			int localCount = 0;

			// Thread-local staging buffer to reduce lock contention on the shared vector.
			thread_local std::vector<std::shared_ptr<GEventDataCollection>> localEvents;
			localEvents.clear();

			while (true) {
				int evn = next.fetch_add(1, std::memory_order_relaxed);
				if (evn > nevents) { break; }

				// Create one event container (factory inserts one dummy hit for detector "ctof").
				auto edc = GEventDataCollection::create(gopt);

				// ---- Extend the event with extra content to exercise the API more thoroughly.

				// (1) Add a second hit under the existing detector ("ctof").
				edc->addDetectorDigitizedData("ctof", GDigitizedData::create(gopt));
				edc->addDetectorTrueInfoData("ctof", GTrueInfoData::create(gopt));

				// (2) Add a second detector key ("ec") with one hit.
				edc->addDetectorDigitizedData("ec", GDigitizedData::create(gopt));
				edc->addDetectorTrueInfoData("ec", GTrueInfoData::create(gopt));

				localEvents.emplace_back(edc);
				++localCount;
			}

			{
				std::scoped_lock lk(collectorMtx);
				for (auto& evt : localEvents) { collected.emplace_back(evt); }
				localEvents.clear();
			}

			log->info(0, "worker ", tid, " processed ", localCount, " events");
		});
	}

	return collected;
}

int main(int argc, char* argv[]) {
	// Aggregate options for event-level data collection.
	auto gopts = std::make_shared<GOptions>(argc, argv, gevent_data::defineOptions());

	// Event-data logger (domain: GEVENTDATA_LOGGER).
	auto log = std::make_shared<GLogger>(gopts, SFUNCTION_NAME, GEVENTDATA_LOGGER);

	// Keep these small by default so the example output remains readable.
	constexpr int nevents  = 5;
	constexpr int nthreads = 4;

	auto events = run_simulation_in_threads(nevents, nthreads, gopts, log);

	// Demonstration: inspect and validate each event container.
	for (const auto& edc : events) {
		if (!edc) continue;
		validate_event_structure(edc, log);
		dump_event(edc, log);
	}

	log->info(0, "Generated ", events.size(), " event containers.");

	return EXIT_SUCCESS;
}
