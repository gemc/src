/**
 * \file gEventDataCollection.cc
 * \brief Implementation of the GEventDataCollection class.
 */

#include "gEventDataCollection.h"
#include "gEventHeader.h"

std::atomic<int> GEventDataCollection::globalEventDataCollectionCounter{1};
std::atomic<int> GEventHeader::globalEventHeaderCounter{1};

void GEventDataCollection::addDetectorTrueInfoData(const std::string& sdName, std::unique_ptr<GTrueInfoData> data) {
	if (gdataCollectionMap.find(sdName) == gdataCollectionMap.end()) { gdataCollectionMap[sdName] = std::make_unique<GDataCollection>(); }
	gdataCollectionMap[sdName]->addTrueInfoData(std::move(data));
	log->info(2, "GEventDataCollection: added new detector TrueInfoData for ", sdName);
}

void GEventDataCollection::addDetectorDigitizedData(const std::string& sdName, std::unique_ptr<GDigitizedData> data) {
	if (gdataCollectionMap.find(sdName) == gdataCollectionMap.end()) { gdataCollectionMap[sdName] = std::make_unique<GDataCollection>(); }
	gdataCollectionMap[sdName]->addDigitizedData(std::move(data));
	log->info(2, "GEventDataCollection: added new detector DigitizedData for ", sdName);
}

