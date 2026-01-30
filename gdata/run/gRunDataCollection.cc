/**
 * \file gRunDataCollection.cc
 * \brief Implementation of the GRunDataCollection class.
 *
 * \details
 * The run data collection integrates (\em accumulates) detector data across many events.
 * It consumes \ref GEventDataCollection instances, and for each detector it merges the
 * event-level per-hit data into a run-level \ref GDataCollection accumulator.
 *
 * Run-level integration is performed by:
 * - \ref GDataCollection::collectTrueInfosData()
 * - \ref GDataCollection::collectDigitizedData()
 */

#include "gRunDataCollection.h"

void GRunDataCollection::collectDetectorTrueInfoData(const std::string& sdName,
                                                     const std::unique_ptr<GTrueInfoData>& data) {
	// Ensure the detector entry exists.
	if (gdataCollectionMap.find(sdName) == gdataCollectionMap.end()) {
		gdataCollectionMap[sdName] = std::make_unique<GDataCollection>();
	}

	// Integrate true-hit data into the run-level accumulator for this detector.
	gdataCollectionMap[sdName]->collectTrueInfosData(std::make_unique<GTrueInfoData>(*data));
	log->info(2, "GRunDataCollection: collected detector TrueInfoData for ", sdName);
}

void GRunDataCollection::collectDetectorDigitizedData(const std::string& sdName,
                                                      const std::unique_ptr<GDigitizedData>& data) {
	if (gdataCollectionMap.find(sdName) == gdataCollectionMap.end()) {
		gdataCollectionMap[sdName] = std::make_unique<GDataCollection>();
	}

	// Integrate digitized-hit data into the run-level accumulator for this detector.
	gdataCollectionMap[sdName]->collectDigitizedData(std::make_unique<GDigitizedData>(*data));
	log->info(2, "GRunDataCollection: collected  detector DigitizedData for ", sdName);
}

void GRunDataCollection::collect_event_data_collection(const std::shared_ptr<GEventDataCollection> edc) {
	// Retrieve the detector map for the event: sdName -> per-detector collection.
	auto& dcm = edc->getDataCollectionMap();

	// Loop over sensitive detectors present in this event.
	for (auto& [sdname, ptr] : dcm) {
		if (!ptr) continue;

		// True-hit entries for this detector in this event:
		// - Event-level: many hits
		// - We integrate each into the run accumulator
		auto& true_infos_data = ptr->getTrueInfoData();
		for (auto& true_info_hit : true_infos_data) {
			collectDetectorTrueInfoData(sdname, true_info_hit);
		}

		// Digitized-hit entries for this detector in this event:
		auto& digitized_data = ptr->getDigitizedData();
		for (auto& digitized_data_hit : digitized_data) {
			collectDetectorDigitizedData(sdname, digitized_data_hit);
		}
	}
}
