/**
* \file gEventDataCollection.cc
 * \brief Implements GEventDataCollection.
 *
 * Non-Doxygen implementation summary:
 * - creates detector entries lazily on first insertion
 * - transfers ownership of truth and digitized hit objects into detector-local containers
 * - defines example/test counters for the event container and event header factories
 */

#include "gEventDataCollection.h"

// Counter reserved for tests and future example helpers.
std::atomic<int> GEventDataCollection::globalEventDataCollectionCounter{1};

// Counter used by GEventHeader::create() to generate unique event numbers in examples/tests.
std::atomic<int> GEventHeader::globalEventHeaderCounter{1};

void GEventDataCollection::addDetectorTrueInfoData(const std::string& sdName, std::unique_ptr<GTrueInfoData> data) {
	// Create the detector entry on first insertion.
	if (gdataCollectionMap.find(sdName) == gdataCollectionMap.end()) {
		gdataCollectionMap[sdName] = std::make_unique<GDataCollection>();
	}

	// Event-level insertion transfers ownership of the hit-side object to the detector container.
	gdataCollectionMap[sdName]->addTrueInfoData(std::move(data));
	log->info(2, "GEventDataCollection: added new detector TrueInfoData for ", sdName);
}

void GEventDataCollection::addDetectorDigitizedData(const std::string& sdName, std::unique_ptr<GDigitizedData> data) {
	// Create the detector entry on first insertion.
	if (gdataCollectionMap.find(sdName) == gdataCollectionMap.end()) {
		gdataCollectionMap[sdName] = std::make_unique<GDataCollection>();
	}

	// Event-level insertion transfers ownership of the hit-side object to the detector container.
	gdataCollectionMap[sdName]->addDigitizedData(std::move(data));
	log->info(2, "GEventDataCollection: added new detector DigitizedData for ", sdName);
}