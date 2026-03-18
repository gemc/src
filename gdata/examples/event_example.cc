/**
 * \file event_example.cc
 * \anchor event_example
 * \brief Event-level example demonstrating how to build and inspect GEventDataCollection objects.
 *
 * \details
 * Summary:
 * - builds event containers
 * - appends truth and digitized hit objects under one or more detector names
 * - inspects the resulting structure, including filtered digitized observables
 *
 * Link to the example overview:
 * - \ref event_example
 */

/**
 * \defgroup gdata_example_event_collection Event data collection example
 * \brief Event-level construction and inspection of per-detector hit data.
 *
 * \details
 * This example emulates a simplified event loop where each event produces hit data for one or more
 * sensitive detectors and stores them into a GEventDataCollection.
 *
 * The GEventDataCollection owns per-hit objects. For each hit, two complementary views may be
 * produced and stored:
 * - GTrueInfoData stores simulation-level truth observables derived from tracking
 * - GDigitizedData stores electronics-level observables produced by detector response and digitization logic
 *
 * For each event, data are organized as:
 * \code
 * sdName -> GDataCollection
 *             - vector<unique_ptr<GTrueInfoData>>   one entry per stored truth hit
 *             - vector<unique_ptr<GDigitizedData>>  one entry per stored digitized hit
 * \endcode
 */

/** @ingroup gdata_example_event_collection
 * \section gdata_event_demo What this example demonstrates
 * - creating event containers with the factory \ref GEventDataCollection::create "create()"
 * - adding additional hits and additional detector keys with:
 *   - \ref GEventDataCollection::addDetectorTrueInfoData "addDetectorTrueInfoData()"
 *   - \ref GEventDataCollection::addDetectorDigitizedData "addDetectorDigitizedData()"
 * - inspecting stored data:
 *   - per-detector hit counts
 *   - identity strings from
 *     \ref GTrueInfoData::getIdentityString "getIdentityString()" and
 *     \ref GDigitizedData::getIdentityString "getIdentityString()"
 *   - truth and digitized observable maps
 * - demonstrating filtering of streaming-readout keys for digitized data with:
 *   - \ref GDigitizedData::getIntObservablesMap "getIntObservablesMap()"
 *   - \ref GDigitizedData::getDblObservablesMap "getDblObservablesMap()"
 *
 * \section gdata_event_threading Threading model
 * The example uses a simple work-distribution pattern:
 * - a shared atomic counter assigns event numbers
 * - each worker thread builds independent GEventDataCollection objects
 * - results are appended to a shared vector under a mutex
 *
 * \note
 * GEventHeader::create(), GTrueInfoData::create(), and GDigitizedData::create()
 * use internal thread-safe counters, so concurrent execution is supported for this example-style workload.
 *
 * \section gdata_event_usage Usage
 * Build this example together with the GData library components and required GEMC utilities
 * such as logging, options, and thread helpers. Run it to print a readable dump of each generated
 * event container.
 *
 * \author \n &copy; Maurizio Ungaro
 * \author e-mail: ungaro@jlab.org
 *
 * @{
 */

// gdata
#include "event/gEventDataCollection.h" // Explicit include for the example entry point.

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
 * \brief Converts a scalar map into a compact deterministic string.
 *
 * \details
 * This helper exists only for examples and tests. It prints key/value pairs in the order provided
 * by the container.
 *
 * Determinism note:
 * - \c std::map iterates in lexicographic key order, so the textual output is stable across runs
 *
 * \tparam MapType A map-like type with string keys and printable scalar values.
 * \param m The map to stringify.
 * \return One single-line representation such as \c "{k1=v1, k2=v2}".
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
 * \brief Logs a human-readable summary of all detector data stored in one event.
 *
 * \details
 * This function performs a deep inspection of the event structure:
 * - loops over detectors in the event data map
 * - prints the number of truth hits and digitized hits
 * - prints identity strings and observables for each stored hit object
 *
 * For digitized data, both filtered views are printed to demonstrate SRO separation:
 * - \c which = 0 : non-SRO observables
 * - \c which = 1 : SRO-only observables
 *
 * The function is intentionally verbose because it is meant to show the structure of the module
 * rather than to be used in production logging.
 *
 * \param edc Event collection to inspect.
 * \param log Logger used to emit formatted messages.
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

		// Truth hits are printed first so the example shows the simulation-side view before the readout-side view.
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

		// Digitized hits are printed with both SRO and non-SRO filtered views.
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

			// Demonstrate the convenience accessor for one common SRO quantity.
			log->info(0, "    timeAtElectronics() = ", dh->getTimeAtElectronics());
		}
	}
}

/**
 * \brief Performs lightweight invariant checks for the generated event structure.
 *
 * \details
 * The checks are intentionally non-fatal so the example can continue and print diagnostics:
 * - the event should contain at least one detector entry
 * - detector collections should not be null
 * - stored hit pointers should not be null
 * - truth and digitized vector sizes are compared for convenience, although the API does not require them to match
 *
 * This is useful when evolving the example or validating future refactoring of container ownership.
 *
 * \param edc Event collection to validate.
 * \param log Logger instance used to report diagnostics.
 */
static void validate_event_structure(const std::shared_ptr<GEventDataCollection>& edc,
									 const std::shared_ptr<GLogger>&              log) {
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

		// Matching truth and digitized counts are often expected by example producers, even though the API allows them to differ.
		if (truthHits.size() != digiHits.size()) {
			log->info(0, "VALIDATION: detector <", sdName, "> truthHits(", truthHits.size(),
					  ") != digitizedHits(", digiHits.size(), ") in event ", edc->getEventNumber());
		}
	}
}

/**
 * \brief Produces a set of events using multiple worker threads.
 *
 * \details
 * Each event is created via \ref GEventDataCollection::create "create()", then extended to demonstrate:
 * - adding additional hits under the same detector key
 * - adding a second detector key to the same event
 *
 * Each thread builds events independently, stores them temporarily in a thread-local vector,
 * and appends them to the shared result vector under a mutex. This keeps synchronization simple
 * while still exercising the example factories in parallel.
 *
 * \param nevents  Total number of events to generate.
 * \param nthreads Number of worker threads to launch.
 * \param gopt     Shared options object.
 * \param log      Logger used for progress messages.
 * \return Vector of generated event containers, one shared pointer per event.
 */
static auto run_simulation_in_threads(int                              nevents,
									  int                              nthreads,
									  const std::shared_ptr<GOptions>& gopt,
									  const std::shared_ptr<GLogger>&  log)
	-> std::vector<std::shared_ptr<GEventDataCollection>> {
	std::mutex                                         collectorMtx;
	std::vector<std::shared_ptr<GEventDataCollection>> collected;
	collected.reserve(static_cast<size_t>(nevents));

	// Thread-safe event counter local to this example run.
	std::atomic<int> next{1};

	// Thread pool. The alias joins on destruction.
	std::vector<jthread_alias> pool;
	pool.reserve(nthreads);

	for (int tid = 0; tid < nthreads; ++tid) {
		pool.emplace_back([&, tid] {
			log->info(0, "worker ", tid, " started");

			int localCount = 0;

			// Thread-local staging buffer reduces lock contention on the shared collector vector.
			thread_local std::vector<std::shared_ptr<GEventDataCollection>> localEvents;
			localEvents.clear();

			while (true) {
				int evn = next.fetch_add(1, std::memory_order_relaxed);
				if (evn > nevents) { break; }

				// Create one event container. The factory inserts one dummy hit for detector "ctof".
				auto edc = GEventDataCollection::create(gopt);

				// Extend the event so the example exercises more than the minimal factory path.

				// Add a second hit under the existing detector key.
				edc->addDetectorDigitizedData("ctof", GDigitizedData::create(gopt));
				edc->addDetectorTrueInfoData("ctof", GTrueInfoData::create(gopt));

				// Add a second detector with one hit pair.
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

/**
 * \brief Entry point for the event example.
 *
 * \details
 * The program:
 * - constructs the option bundle for event-level data containers
 * - creates a logger
 * - generates a small set of example events in parallel
 * - validates each event structure
 * - prints a readable dump of each event
 *
 * The event count is intentionally small so the output remains easy to inspect manually.
 *
 * \param argc Argument count forwarded to GOptions.
 * \param argv Argument vector forwarded to GOptions.
 * \return \c EXIT_SUCCESS on normal completion.
 */
int main(int argc, char* argv[]) {
	// Aggregate options for event-level data collection.
	auto gopts = std::make_shared<GOptions>(argc, argv, gevent_data::defineOptions());

	// Event-data logger.
	auto log = std::make_shared<GLogger>(gopts, SFUNCTION_NAME, GEVENTDATA_LOGGER);

	// Keep these small by default so the example output remains readable.
	constexpr int nevents  = 5;
	constexpr int nthreads = 4;

	auto events = run_simulation_in_threads(nevents, nthreads, gopts, log);

	// Inspect and validate each generated event container.
	for (const auto& edc : events) {
		if (!edc) continue;
		validate_event_structure(edc, log);
		dump_event(edc, log);
	}

	log->info(0, "Generated ", events.size(), " event containers.");

	return EXIT_SUCCESS;
}

/**
 * @}
 */