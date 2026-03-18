/**
 * \file gRunDataCollection.cc
 * \brief Implementation of GRunDataCollection.
 *
 * \details
 * The header gRunDataCollection.h contains the authoritative API documentation, including
 * current integration semantics and merge behavior.
 */

#include "gRunDataCollection.h"

void GRunDataCollection::collectDetectorDigitizedData(const std::string&                     sdName,
													  const std::unique_ptr<GDigitizedData>& data) {
	// Create the detector accumulator entry on first use.
	if (gdataCollectionMap.find(sdName) == gdataCollectionMap.end()) {
		gdataCollectionMap[sdName] = std::make_unique<GDataCollection>();
	}

	// Delegate detector-local digitized accumulation.
	gdataCollectionMap[sdName]->collectDigitizedData(data);
}

void GRunDataCollection::collect_event_data_collection(const std::shared_ptr<GEventDataCollection> edc) {
	// Count the integrated event once at the run-summary level.
	grun_header->increment_events_processed();

	// Detector map for this event: detector name -> per-detector event data.
	auto& dcm = edc->getDataCollectionMap();

	// Integrate the digitized hit entries present for each detector.
	for (auto& [sdname, ptr] : dcm) {
		if (!ptr) continue;

		auto& digitized_data = ptr->getDigitizedData();
		for (auto& digitized_data_hit : digitized_data) {
			collectDetectorDigitizedData(sdname, digitized_data_hit);
		}
	}
}

void GRunDataCollection::collect_event_data_collections(const std::string&              sdName,
														std::unique_ptr<GDigitizedData> ddata) {
	collectDetectorDigitizedData(sdName, ddata);
	log->info(2, *gdataCollectionMap[sdName]);
}

void GRunDataCollection::merge(const GRunDataCollection& other) {
	// Merge run-level counters first.
	grun_header->add_events_processed(other.get_events_processed());
	grun_header->add_events_with_payload(other.get_events_with_payload());

	const auto& other_map = other.getDataCollectionMap();

	// Merge already-integrated digitized detector content from the other accumulator.
	for (const auto& [sdName, other_data_collection] : other_map) {
		if (!other_data_collection) {
			continue;
		}

		const auto& other_digitized_data = other_data_collection->getDigitizedData();
		for (const auto& digitized_data_hit : other_digitized_data) {
			if (digitized_data_hit) {
				collectDetectorDigitizedData(sdName, digitized_data_hit);
			}
		}
	}
}