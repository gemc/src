/**
* \file gEventDataCollection.cc
 * \brief Implementation of \ref GEventDataCollection.
 *
 * \details
 * See \ref gEventDataCollection.h for the authoritative API documentation and ownership rules.
 */

// See header for API docs.

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
	if (gdataCollectionMap.find(sdName) == gdataCollectionMap.end()) {
		gdataCollectionMap[sdName] = std::make_unique<GDataCollection>();
	}

	// Event-level: store a new hit entry (ownership transferred).
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
