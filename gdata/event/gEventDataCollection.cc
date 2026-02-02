/**
 * \file gEventDataCollection.cc
 * \brief Implementation of the \ref GEventDataCollection class.
 *
 * \details
 * An event collection owns per-hit objects for each sensitive detector and provides
 * the append API used during event processing.
 *
 * Key behaviors:
 * - Detector entries are created on demand when first referenced by name.
 * - Hit objects are owned via \c std::unique_ptr and are moved into the collection.
 * - The class does not enforce policies such as "truth and digitized hit counts must match";
 *   such validation is typically performed by upstream logic or examples/tests.
 *
 * Ownership summary:
 * - After calling \ref GEventDataCollection::addDetectorTrueInfoData "addDetectorTrueInfoData()"
 *   or \ref GEventDataCollection::addDetectorDigitizedData "addDetectorDigitizedData()",
 *   the event container owns the provided object and the caller must not use the moved-from pointer.
 */

#include "gEventDataCollection.h"

/// Counter used by examples/tests (not currently used in create()).
///
/// \details
/// This exists as a convenience hook for potential future example factories.
/// Current example behavior uses \ref GEventHeader::create "create()" as the event counter.
std::atomic<int> GEventDataCollection::globalEventDataCollectionCounter{1};

/// Counter used by \ref GEventHeader::create "create()" to generate unique event numbers in examples/tests.
std::atomic<int> GEventHeader::globalEventHeaderCounter{1};

void GEventDataCollection::addDetectorTrueInfoData(const std::string& sdName, std::unique_ptr<GTrueInfoData> data) {
	// Create detector entry if it does not exist.
	//
	// This makes the API robust for detectors appearing only in certain events.
	if (gdataCollectionMap.find(sdName) == gdataCollectionMap.end()) {
		gdataCollectionMap[sdName] = std::make_unique<GDataCollection>();
	}

	// Event-level: store a new hit entry (ownership transferred).
	//
	// After std::move(data), the caller relinquishes ownership and should not use 'data'.
	gdataCollectionMap[sdName]->addTrueInfoData(std::move(data));
	log->info(2, "GEventDataCollection: added new detector TrueInfoData for ", sdName);
}

void GEventDataCollection::addDetectorDigitizedData(const std::string& sdName, std::unique_ptr<GDigitizedData> data) {
	// Ensure the per-detector container exists.
	if (gdataCollectionMap.find(sdName) == gdataCollectionMap.end()) {
		gdataCollectionMap[sdName] = std::make_unique<GDataCollection>();
	}

	// Event-level: store a new hit entry (ownership transferred).
	gdataCollectionMap[sdName]->addDigitizedData(std::move(data));
	log->info(2, "GEventDataCollection: added new detector DigitizedData for ", sdName);
}
