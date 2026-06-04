/**
 * \file run_example.cc
 * \anchor run_example
 * \brief Example demonstrating run-level integration of event data.
 *
 * \details
 * Summary:
 * - generates multiple event containers
 * - integrates them into one run summary
 * - compares accumulated values against independently computed reference sums
 *
 * Link to the example overview:
 * - \ref run_example
 */

/**
 * \defgroup gdata_example_run_integration Run data integration example
 * \brief Run-level integration of many events into a single detector-summary map.
 *
 * \details
 * This example shows how to integrate many GEventDataCollection objects into one
 * GRunDataCollection summary using accumulation semantics based on sums of scalar observables.
 *
 * Workflow:
 * 1. Build many events, optionally in parallel.
 * 2. Integrate each event into a GRunDataCollection through
 *    \ref GRunDataCollection::collect_event_data_collection "collect_event_data_collection()".
 * 3. Inspect the resulting detector summary and compare it to reference sums.
 */

/** @ingroup gdata_example_run_integration
 * \section gdata_run_demo What this example demonstrates
 * - creating a set of events with multiple detectors and multiple hits per detector
 * - run-level accumulation that produces sums of observables across all events and hits
 * - a reference self-check that compares direct sums from events against the run accumulator
 *
 * \section gdata_run_integration_details Integration details worth noticing
 * - truth integration checks sums of truth doubles stored in GTrueInfoData
 * - digitized integration checks sums of non-SRO digitized keys only
 * - non-SRO filtering is obtained with:
 *   - \ref GDigitizedData::getIntObservablesMap "getIntObservablesMap()"
 *   - \ref GDigitizedData::getDblObservablesMap "getDblObservablesMap()"
 *   using \c which = 0
 *
 * \section gdata_run_threading Threading model
 * Event generation can be parallel. Run integration is kept sequential in this example so:
 * - the reference check remains straightforward
 * - no separate merge API is needed for multiple partial run accumulators inside the example itself
 *
 * \warning
 * Accumulated values are sums. If averages, rates, or normalized observables are needed,
 * they must be computed by downstream consumer code.
 *
 * \section gdata_run_usage Usage
 * Build this example together with the GData library components and required GEMC utilities.
 *
 * \author \n &copy; Maurizio Ungaro
 * \author e-mail: ungaro@jlab.org
 *
 * @{
 */

// gdata
#include "run/gRunDataCollection.h"
#include "event/gEventDataCollection.h"

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
 * This helper is used for example output only.
 * The ordering is stable because \c std::map iterates in lexicographic key order.
 *
 * \tparam MapType A map-like type with string keys and printable scalar values.
 * \param m The map to stringify.
 * \return A string such as \c "{k1=v1, k2=v2}".
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
 * \brief Per-detector map of floating-point reference sums used by the example.
 *
 * \details
 * Outer key:
 * - detector name
 *
 * Inner key:
 * - observable name
 *
 * Value:
 * - accumulated floating-point sum
 */
using PerDetectorDoubles = std::map<std::string, std::map<std::string, double>>;

/**
 * \brief Per-detector map of integer-like reference sums used by the example.
 *
 * \details
 * Integer observables are accumulated into \c long long values so the example can compare
 * totals without depending on narrower integer types.
 */
using PerDetectorInts = std::map<std::string, std::map<std::string, long long>>;

/**
 * \brief Generates a set of event containers using multiple worker threads.
 *
 * \details
 * Each generated event starts from the example factory and is then extended so integration
 * exercises multiple detectors and multiple hits per detector.
 *
 * A thread-local staging vector is used to reduce lock contention on the shared output vector.
 *
 * \param nevents  Total number of events to generate.
 * \param nthreads Number of worker threads to launch.
 * \param gopt     Shared options object.
 * \param log      Logger used for progress messages.
 * \return Vector containing all generated event collections.
 */
static auto generate_events_in_threads(int                              nevents,
									   int                              nthreads,
									   const std::shared_ptr<GOptions>& gopt,
									   const std::shared_ptr<GLogger>&  log)
	-> std::vector<std::shared_ptr<GEventDataCollection>> {
	std::mutex                                         collectorMtx;
	std::vector<std::shared_ptr<GEventDataCollection>> collected;
	collected.reserve(static_cast<size_t>(nevents));

	std::atomic<int> next{1};

	std::vector<jthread_alias> pool;
	pool.reserve(nthreads);

	for (int tid = 0; tid < nthreads; ++tid) {
		pool.emplace_back([&, tid] {
			log->info(0, "worker ", tid, " started");

			int                                                             localCount = 0;
			thread_local std::vector<std::shared_ptr<GEventDataCollection>> localEvents;
			localEvents.clear();

			while (true) {
				int evn = next.fetch_add(1, std::memory_order_relaxed);
				if (evn > nevents) break;

				auto edc = GEventDataCollection::create(gopt);

				// Extend the event so integration covers more than the minimal factory-generated content.
				edc->addDetectorDigitizedData("ctof", GDigitizedData::create(gopt));
				edc->addDetectorTrueInfoData("ctof", GTrueInfoData::create(gopt));

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
 * \brief Computes reference sums directly from the generated event containers.
 *
 * \details
 * This function provides an independent baseline used to validate run integration.
 *
 * Accumulated channels:
 * - truth doubles from all truth hits
 * - digitized integers from non-SRO keys only
 * - digitized doubles from non-SRO keys only
 *
 * The resulting maps are later compared against the integrated content stored in GRunDataCollection.
 *
 * \param events        Event containers to scan.
 * \param truth_ref     Output map receiving truth-double sums.
 * \param digi_int_ref  Output map receiving digitized integer sums.
 * \param digi_dbl_ref  Output map receiving digitized floating-point sums.
 */
static void compute_reference_sums(const std::vector<std::shared_ptr<GEventDataCollection>>& events,
								   PerDetectorDoubles&                                       truth_ref,
								   PerDetectorInts&                                          digi_int_ref,
								   PerDetectorDoubles&                                       digi_dbl_ref) {
	for (const auto& edc : events) {
		if (!edc) continue;

		const auto& dcm = edc->getDataCollectionMap();
		for (const auto& [sdName, det] : dcm) {
			if (!det) continue;

			// Truth-side accumulation: all numeric truth observables are summed directly.
			for (const auto& th : det->getTrueInfoData()) {
				if (!th) continue;
				for (const auto& [k, v] : th->getDoubleVariablesMap()) {
					truth_ref[sdName][k] += v;
				}
			}

			// Digitized-side accumulation: only non-SRO observables are part of the reference sums.
			for (const auto& dh : det->getDigitizedData()) {
				if (!dh) continue;

				for (const auto& [k, v] : dh->getIntObservablesMap(0)) {
					digi_int_ref[sdName][k] += static_cast<long long>(v);
				}
				for (const auto& [k, v] : dh->getDblObservablesMap(0)) {
					digi_dbl_ref[sdName][k] += v;
				}
			}
		}
	}
}

/**
 * \brief Integrates all generated events into one run container.
 *
 * \details
 * The example constructs a run header with run number 1, then forwards each event into the
 * run collection using the standard event-integration API.
 *
 * \param events Event collections to integrate.
 * \param gopt   Shared options object.
 * \return Shared pointer to the integrated run collection.
 */
static auto integrate_into_run(const std::vector<std::shared_ptr<GEventDataCollection>>& events,
							   const std::shared_ptr<GOptions>&                          gopt)
	-> std::shared_ptr<GRunDataCollection> {
	auto grun_header = std::make_unique<GRunHeader>(gopt, 1); // Run id 1, thread id default -1.
	auto run_data    = std::make_shared<GRunDataCollection>(gopt, std::move(grun_header));

	for (const auto& edc : events) {
		if (!edc) continue;
		run_data->collect_event_data_collection(edc);
	}

	return run_data;
}

/**
 * \brief Compares the run-integrated container against independently computed reference sums.
 *
 * \details
 * The function prints:
 * - the integrated truth map for each detector
 * - the integrated digitized non-SRO maps
 * - the digitized SRO maps for inspection
 * - mismatch lines whenever an integrated value differs from its reference value
 *
 * This is intended as a validation and demonstration utility rather than a generic testing framework.
 *
 * \param run_data     Run collection to validate.
 * \param truth_ref    Reference sums for truth doubles.
 * \param digi_int_ref Reference sums for digitized integer observables.
 * \param digi_dbl_ref Reference sums for digitized floating-point observables.
 * \param log          Logger used to print the validation summary.
 */
static void validate_run_against_reference(const std::shared_ptr<GRunDataCollection>& run_data,
										   const PerDetectorDoubles&                  truth_ref,
										   const PerDetectorInts&                     digi_int_ref,
										   const PerDetectorDoubles&                  digi_dbl_ref,
										   const std::shared_ptr<GLogger>&            log) {
	log->info(0, "============================================================");
	log->info(0, "RUN SUMMARY (integrated): runID=", run_data->getRunNumber());
	log->info(0, "============================================================");

	const auto& rmap = run_data->getDataCollectionMap();
	if (rmap.empty()) {
		log->info(0, "Run data map is empty (no detectors integrated).");
		return;
	}

	for (const auto& [sdName, det] : rmap) {
		if (!det) continue;

		const auto& truthVec = det->getTrueInfoData();
		const auto& digiVec  = det->getDigitizedData();

		log->info(0, "Detector <", sdName, ">: integrated truth entries=", truthVec.size(),
				  " integrated digitized entries=", digiVec.size());

		// Truth integrated entry.
		if (!truthVec.empty() && truthVec.front()) {
			const auto integrated_truth = truthVec.front()->getDoubleVariablesMap();
			log->info(0, "  integrated truth doubles: ", map_to_string(integrated_truth));

			const auto it_ref_det = truth_ref.find(sdName);
			if (it_ref_det != truth_ref.end()) {
				for (const auto& [k, refv] : it_ref_det->second) {
					const auto   itv = integrated_truth.find(k);
					const double got = (itv == integrated_truth.end()) ? 0.0 : itv->second;
					if (got != refv) {
						log->info(0, "  MISMATCH truth <", sdName, ">::", k, " got=", got, " ref=", refv);
					}
				}
			}
			else {
				log->info(0, "  NOTE: no reference truth sums found for detector <", sdName, ">.");
			}
		}
		else {
			log->info(0, "  integrated truth: <none>");
		}

		// Digitized integrated entry.
		if (!digiVec.empty() && digiVec.front()) {
			const auto ints_non_sro = digiVec.front()->getIntObservablesMap(0);
			const auto dbls_non_sro = digiVec.front()->getDblObservablesMap(0);

			log->info(0, "  integrated digi int  non-SRO: ", map_to_string(ints_non_sro));
			log->info(0, "  integrated digi dbl  non-SRO: ", map_to_string(dbls_non_sro));

			const auto ints_sro = digiVec.front()->getIntObservablesMap(1);
			const auto dbls_sro = digiVec.front()->getDblObservablesMap(1);
			log->info(0, "  integrated digi int  SRO:     ", map_to_string(ints_sro));
			log->info(0, "  integrated digi dbl  SRO:     ", map_to_string(dbls_sro));

			const auto it_int_ref_det = digi_int_ref.find(sdName);
			if (it_int_ref_det != digi_int_ref.end()) {
				for (const auto& [k, refv] : it_int_ref_det->second) {
					const auto      itv = ints_non_sro.find(k);
					const long long got = (itv == ints_non_sro.end()) ? 0LL : static_cast<long long>(itv->second);
					if (got != refv) {
						log->info(0, "  MISMATCH digi-int <", sdName, ">::", k, " got=", got, " ref=", refv);
					}
				}
			}
			else {
				log->info(0, "  NOTE: no reference digitized-int sums found for detector <", sdName, ">.");
			}

			const auto it_dbl_ref_det = digi_dbl_ref.find(sdName);
			if (it_dbl_ref_det != digi_dbl_ref.end()) {
				for (const auto& [k, refv] : it_dbl_ref_det->second) {
					const auto   itv = dbls_non_sro.find(k);
					const double got = (itv == dbls_non_sro.end()) ? 0.0 : itv->second;
					if (got != refv) {
						log->info(0, "  MISMATCH digi-dbl <", sdName, ">::", k, " got=", got, " ref=", refv);
					}
				}
			}
			else {
				log->info(0, "  NOTE: no reference digitized-double sums found for detector <", sdName, ">.");
			}
		}
		else {
			log->info(0, "  integrated digitized: <none>");
		}
	}
}

/**
 * \brief Entry point for the run-integration example.
 *
 * \details
 * The program:
 * - builds the aggregated run-level option bundle
 * - generates a small set of events
 * - computes direct reference sums from those events
 * - integrates the same events into a run container
 * - validates the integrated result against the references
 *
 * \param argc Argument count forwarded to GOptions.
 * \param argv Argument vector forwarded to GOptions.
 * \return \c EXIT_SUCCESS on normal completion.
 */
int main(int argc, char* argv[]) {
	auto gopts = std::make_shared<GOptions>(argc, argv, grun_data::defineOptions());
	auto log   = std::make_shared<GLogger>(gopts, SFUNCTION_NAME, GRUNDATA_LOGGER);

	constexpr int nevents  = 20;
	constexpr int nthreads = 4;

	auto events = generate_events_in_threads(nevents, nthreads, gopts, log);
	log->info(0, "Generated ", events.size(), " events.");

	PerDetectorDoubles truth_ref;
	PerDetectorInts    digi_int_ref;
	PerDetectorDoubles digi_dbl_ref;
	compute_reference_sums(events, truth_ref, digi_int_ref, digi_dbl_ref);

	auto runData = integrate_into_run(events, gopts);
	validate_run_against_reference(runData, truth_ref, digi_int_ref, digi_dbl_ref, log);

	return EXIT_SUCCESS;
}

/** @} */