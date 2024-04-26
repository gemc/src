// gdata
#include "gDataCollection.h"

// public interface to add hits

// true infos
void GDataCollection::addTrueInfoData(GTrueInfoData *data) {
	trueInfosData->push_back(data);
}

// digitized hit
void GDataCollection::addDigitizedData(GDigitizedData *data) {
	digitizedData->push_back(data);
}

