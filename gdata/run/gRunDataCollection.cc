/**
 * \file gRunDataCollection.cc
 * \brief Implementation of the \ref GRunDataCollection class.
 *
 * \details
 * The run data collection integrates (accumulates) detector data across many events.
 * It consumes \ref GEventDataCollection instances, and for each detector it merges the
 * event-level per-hit data into a run-level \ref GDataCollection accumulator.
 *
 * Run-level integration is performed by delegating to \ref GDataCollection :
 * - \ref GDataCollection::collectTrueInfosData "collectTrueInfosData()" for truth hits
 * - \ref GDataCollection::collectDigitizedData "collectDigitizedData()" for digitized hits
 *
 * Integration behavior summary:
 * - The first integrated hit creates the accumulator entry for that detector.
 * - Subsequent hits contribute by summation of scalar observables (see \ref GDataCollection docs).
 *
 * \note Threading
 * This file does not implement a cross-thread merge strategy. If multiple run accumulators are
 * produced (e.g. one per worker thread), merging strategies must be implemented at a higher level.
 */

#include "gRunDataCollection.h"

void GRunDataCollection::collectDetectorTrueInfoData(const std::string&                    sdName,
                                                     const std::unique_ptr<GTrueInfoData>& data) {
	// Ensure the detector entry exists.
	//
	// Rationale:
	// - The run accumulator is keyed by detector name. If the detector appears for the first time
	//   in any event, we allocate its accumulator on demand.
	if (gdataCollectionMap.find(sdName) == gdataCollectionMap.end()) {
		gdataCollectionMap[sdName] = std::make_unique<GDataCollection>();
	}

	// Integrate true-hit data into the run-level accumulator for this detector.
	//
	// Implementation detail:
	// - We deep-copy \p data into a temporary \ref GTrueInfoData to decouple from the event container's
	//   ownership and lifetime.
	// - \ref GDataCollection::collectTrueInfosData "collectTrueInfosData()" will store the first copy
	//   and then accumulate subsequent hits by summing doubles via
	//   \ref GTrueInfoData::accumulateVariable "accumulateVariable()".
	gdataCollectionMap[sdName]->collectTrueInfosData(std::make_unique<GTrueInfoData>(*data));
	log->info(2, "GRunDataCollection: collected detector TrueInfoData for ", sdName);
}

void GRunDataCollection::collectDetectorDigitizedData(const std::string&                     sdName,
                                                      const std::unique_ptr<GDigitizedData>& data) {
	// Ensure the detector entry exists, same as for truth data.
	if (gdataCollectionMap.find(sdName) == gdataCollectionMap.end()) {
		gdataCollectionMap[sdName] = std::make_unique<GDataCollection>();
	}

	// Integrate digitized-hit data into the run-level accumulator for this detector.
	//
	// Filtering behavior:
	// - \ref GDataCollection::collectDigitizedData "collectDigitizedData()" reads non-SRO keys only
	//   (\c which=0) and then accumulates scalars via
	//   \ref GDigitizedData::accumulateVariable "accumulateVariable()".
	gdataCollectionMap[sdName]->collectDigitizedData(std::make_unique<GDigitizedData>(*data));
	log->info(2, "GRunDataCollection: collected  detector DigitizedData for ", sdName);
}

void GRunDataCollection::collect_event_data_collection(const std::shared_ptr<GEventDataCollection> edc) {
	// Retrieve the detector map for the event: sdName -> per-detector collection.
	//
	// The event container holds a per-detector \ref GDataCollection, each of which holds vectors:
	// - vector<unique_ptr<GTrueInfoData>>
	// - vector<unique_ptr<GDigitizedData>>
	//
	// Note:
	// - This method integrates *every hit* present in the event container, for each detector key.
	auto& dcm = edc->getDataCollectionMap();

	// Loop over sensitive detectors present in this event.
	for (auto& [sdname, ptr] : dcm) {
		if (!ptr) continue;

		// True-hit entries for this detector in this event:
		// - Event-level: many hits
		// - Run-level: integrate each hit into the per-detector accumulator
		auto& true_infos_data = ptr->getTrueInfoData();
		for (auto& true_info_hit : true_infos_data) {
			collectDetectorTrueInfoData(sdname, true_info_hit);
		}

		// Digitized-hit entries for this detector in this event:
		// - Same pattern as truth: integrate each hit into the run accumulator
		auto& digitized_data = ptr->getDigitizedData();
		for (auto& digitized_data_hit : digitized_data) {
			collectDetectorDigitizedData(sdname, digitized_data_hit);
		}
	}
}
