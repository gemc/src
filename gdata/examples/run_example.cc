/**
 * \file run_example.cc
 * \brief Example demonstrating run-level integration (accumulation) of event data.
 */

/**
 * \defgroup gdata_run_example Run data integration example
 *
 * \brief Run-level integration of many events into a single detector-summary map.
 *
 * \details
 * This example shows how to integrate many \ref GEventDataCollection objects into a single
 * \ref GRunDataCollection summary using accumulation semantics (sums of scalar observables).
 *
 * The workflow is:
 * 1) Build N events (\ref GEventDataCollection) — possibly in parallel.
 * 2) Integrate each event into a \ref GRunDataCollection via
 *    \ref GRunDataCollection::collect_event_data_collection "collect_event_data_collection()".
 * 3) Inspect the resulting run summary map:
 *    \code
 *      sdName -> GDataCollection (run-integrated)
 *                - vector<unique_ptr<GTrueInfoData>>   typically size 1 (integrated entry)
 *                - vector<unique_ptr<GDigitizedData>>  typically size 1 (integrated entry)
 *    \endcode
 *
 * \section gdata_run_demo What this example demonstrates
 * - Creating a set of events with multiple detectors and multiple hits per detector.
 * - Run-level integration that produces *sums* of observables across all events/hits.
 * - A reference self-check:
 *   - compute sums directly by scanning the generated events
 *   - compare those sums to what \ref GRunDataCollection produced
 *
 * \section gdata_run_integration_details Integration details worth noticing
 * - Truth integration: this example checks sums of truth doubles (as stored by \ref GTrueInfoData).
 * - Digitized integration: this example checks sums of *non-SRO* digitized keys only.
 *   Filtering is applied by calling:
 *   - \ref GDigitizedData::getIntObservablesMap "getIntObservablesMap(0)"
 *   - \ref GDigitizedData::getDblObservablesMap "getDblObservablesMap(0)"
 *   where \c which=0 means "exclude SRO keys".
 *
 * \section gdata_run_threading Threading model
 * Event generation can be parallel. Run integration is performed sequentially in this example to:
 * - keep the reference check straightforward
 * - avoid requiring a merge API for multiple run accumulators
 *
 * \warning Accumulated values are sums. If you need averages/rates, normalize in the consumer.
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
 * \brief Convert a scalar map into a compact string for logging.
 *
 * \details
 * This helper is used for example output only.
 *
 * Determinism note:
 * - \c std::map iterates in lexicographic key order, which makes the output stable across runs.
 *
 * \tparam MapType A map-like type with string keys and printable scalar values.
 * \param m The map to stringify.
 * \return A string like "{k1=v1, k2=v2}".
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
 * \brief Type alias for per-detector scalar accumulation maps used in this example.
 *
 * \details
 * We keep separate reference sums for:
 * - truth doubles (numeric observables stored in \ref GTrueInfoData)
 * - digitized ints  (non-SRO keys only)
 * - digitized dbls  (non-SRO keys only)
 *
 * Using distinct maps makes it explicit which channels are expected to be integrated by which rules.
 */
using PerDetectorDoubles = std::map<std::string, std::map<std::string, double>>;
using PerDetectorInts    = std::map<std::string, std::map<std::string, long long>>;

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

				// Extend the event to ensure integration exercises multiple detectors/hits.
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

static void compute_reference_sums(const std::vector<std::shared_ptr<GEventDataCollection>>& events,
                                   PerDetectorDoubles&                                       truth_ref,
                                   PerDetectorInts&                                          digi_int_ref,
                                   PerDetectorDoubles&                                       digi_dbl_ref) {
	for (const auto& edc : events) {
		if (!edc) continue;

		const auto& dcm = edc->getDataCollectionMap();
		for (const auto& [sdName, det] : dcm) {
			if (!det) continue;

			// Truth: doubles
			for (const auto& th : det->getTrueInfoData()) {
				if (!th) continue;
				for (const auto& [k, v] : th->getDoubleVariablesMap()) {
					truth_ref[sdName][k] += v;
				}
			}

			// Digitized: ints and doubles (non-SRO only, which=0)
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

static auto integrate_into_run(const std::vector<std::shared_ptr<GEventDataCollection>>& events,
                               const std::shared_ptr<GOptions>&                          gopt)
	-> std::shared_ptr<GRunDataCollection> {
	auto grun_header = std::make_unique<GRunHeader>(gopt, 1); // run id 1, thread id default -1
	auto run_data    = std::make_shared<GRunDataCollection>(gopt, std::move(grun_header));

	for (const auto& edc : events) {
		if (!edc) continue;
		run_data->collect_event_data_collection(edc);
	}

	return run_data;
}

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

		// ---- Truth integrated entry
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

		// ---- Digitized integrated entry
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
