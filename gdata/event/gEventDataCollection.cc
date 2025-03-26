// gdata 
#include "gEventDataCollection.h"


void GEventDataCollection::addDetectorTrueInfoData(string sdName, GTrueInfoData *data) {

	// digitized data not found, creating it
	if ( gdataCollectionMap->find(sdName) == gdataCollectionMap->end() ) {
		(*gdataCollectionMap)[sdName] = new GDataCollection();
	}
	(*gdataCollectionMap)[sdName]->addTrueInfoData(data);
	log.info(2, "GEventDataCollection: added new detector TrueInfoData for ", sdName);
}

void GEventDataCollection::addDetectorDigitizedData(string sdName, GDigitizedData *data) {

	// digitized data not found, creating it
	if ( gdataCollectionMap->find(sdName) == gdataCollectionMap->end() ) {
		(*gdataCollectionMap)[sdName] = new GDataCollection();
	}
	(*gdataCollectionMap)[sdName]->addDigitizedData(data);
	log.info(2, "GEventDataCollection: added new detector DigitizedData for ", sdName);
}


const vector<GTrueInfoData*>* GEventDataCollection::getTrueInfoDataForDetector(string detector) const {

	if ( gdataCollectionMap->find(detector) != gdataCollectionMap->end() ) {
		return (*gdataCollectionMap)[detector]->getTrueInfoData();
	}

	return nullptr;
}


const vector<GDigitizedData*>* GEventDataCollection::getDigitizedDataForDetector(string detector) const {

	if ( gdataCollectionMap->find(detector) != gdataCollectionMap->end() ) {
		return (*gdataCollectionMap)[detector]->getDigitizedData();
	}

	return nullptr;
}
