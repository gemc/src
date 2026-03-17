/**
 * \file gRunDataCollection.cc
 * \brief Implementation of \ref GRunDataCollection.
 *
 * \details
 * See \ref gRunDataCollection.h for the authoritative API documentation and integration semantics.
 */

// See header for API docs.

#include "gRunDataCollection.h"

void GRunDataCollection::collectDetectorDigitizedData(const std::string&                     sdName,
													  const std::unique_ptr<GDigitizedData>& data) {
	// Ensure the detector entry exists, same as for truth data.
	if (gdataCollectionMap.find(sdName) == gdataCollectionMap.end()) {
		gdataCollectionMap[sdName] = std::make_unique<GDataCollection>();
	}

	// Integrate digitized-hit data into the run-level accumulator for this detector.
	gdataCollectionMap[sdName]->collectDigitizedData(data);
}

void GRunDataCollection::collect_event_data_collection(const std::shared_ptr<GEventDataCollection> edc) {
	// Count one integrated event.
	grun_header->increment_events_processed();

	// Retrieve the detector map for the event: sdName -> per-detector collection.
	auto& dcm = edc->getDataCollectionMap();

	// Loop over sensitive detectors present in this event.
	for (auto& [sdname, ptr] : dcm) {
		if (!ptr) continue;

		// Digitized-hit entries for this detector in this event.
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

	grun_header->add_events_processed(other.get_events_processed());
	grun_header->add_events_with_payload(other.get_events_with_payload());

	const auto& other_map = other.getDataCollectionMap();

	for (const auto& [sdName, other_data_collection] : other_map) {
		if (!other_data_collection) {
			continue;
		}

		// Merge already-integrated digitized data for this detector.
		const auto& other_digitized_data = other_data_collection->getDigitizedData();
		for (const auto& digitized_data_hit : other_digitized_data) {
			if (digitized_data_hit) {
				collectDetectorDigitizedData(sdName, digitized_data_hit);
			}
		}
	}
}