/**
* \file gEventDataCollection.cc
 * \brief Implementation of the GEventDataCollection class.
 *
 * \details
 * Event collections own per-hit objects for each sensitive detector and provide
 * a simple API to append hit data during event processing.
 */

#include "gEventDataCollection.h"

/// Counter used by examples/tests (not currently used in create()).
std::atomic<int> GEventDataCollection::globalEventDataCollectionCounter{1};

/// Counter used by \ref GEventHeader::create() to generate unique event numbers in examples/tests.
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
	if (gdataCollectionMap.find(sdName) == gdataCollectionMap.end()) {
		gdataCollectionMap[sdName] = std::make_unique<GDataCollection>();
	}

	// Event-level: store a new hit entry (ownership transferred).
	gdataCollectionMap[sdName]->addDigitizedData(std::move(data));
	log->info(2, "GEventDataCollection: added new detector DigitizedData for ", sdName);
}
