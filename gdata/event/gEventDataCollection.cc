/**
 * \file GEventDataCollection.cpp
 * \brief Implementation of the GEventDataCollection class.
 */

#include "gEventDataCollection.h"

void GEventDataCollection::addDetectorTrueInfoData(std::string sdName, GTrueInfoData* data) {
	if (gdataCollectionMap.find(sdName) == gdataCollectionMap.end()) { gdataCollectionMap[sdName] = std::make_unique<GDataCollection>(log); }
	gdataCollectionMap[sdName]->addTrueInfoData(data);
	log->info(2, "GEventDataCollection: added new detector TrueInfoData for ", sdName);
}

void GEventDataCollection::addDetectorDigitizedData(std::string sdName, GDigitizedData* data) {
	if (gdataCollectionMap.find(sdName) == gdataCollectionMap.end()) { gdataCollectionMap[sdName] = std::make_unique<GDataCollection>(log); }
	gdataCollectionMap[sdName]->addDigitizedData(data);
	log->info(2, "GEventDataCollection: added new detector DigitizedData for ", sdName);
}


const std::vector<GTrueInfoData*>* GEventDataCollection::getTrueInfoDataForDetector(std::string detector) const {
	auto it = gdataCollectionMap.find(detector);
	if (it != gdataCollectionMap.end()) { return it->second->getTrueInfoData(); }
	return nullptr;
}

const std::vector<GDigitizedData*>* GEventDataCollection::getDigitizedDataForDetector(std::string detector) const {
	auto it = gdataCollectionMap.find(detector);
	if (it != gdataCollectionMap.end()) { return it->second->getDigitizedData(); }
	return nullptr;
}
