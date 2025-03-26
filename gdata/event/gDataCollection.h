#ifndef GHITSCOLLECTION_H
#define GHITSCOLLECTION_H 1

// gdata
#include "gTrueInfoData.h"
#include "gDigitizedData.h"

// observable types include int, float, double,
#include <string>
#include <map>
#include <vector>


// collected for each sensitive detector
class GDataCollection {

public:
	GDataCollection(GLogger * const logger) : log(logger) {

		log->debug(CONSTRUCTOR, "GDataCollection");
		trueInfosData = new vector<GTrueInfoData *>;
		digitizedData = new vector<GDigitizedData *>;
	}

	~GDataCollection() {

		for (auto *hit: (*trueInfosData)) { delete hit; }
		for (auto *hit: (*digitizedData)) { delete hit; }
		log->debug(DESTRUCTOR, "GDataCollection");

		delete trueInfosData;
		delete digitizedData;
	}


	// public interface to add hit
	void addTrueInfoData(GTrueInfoData *data) {
		log->debug(NORMAL, " adding hit to trueInfosData with identity: ", data->getIdentityString());
		trueInfosData->push_back(data);
	}

	void addDigitizedData(GDigitizedData *data) {
		log->debug(NORMAL, " adding hit to digitizedData with identity: ", data->getIdentityString());
		digitizedData->push_back(data);
	}

	// one entry / hit
	inline const vector<GTrueInfoData *> *getTrueInfoData() const { return trueInfosData; }

	inline const vector<GDigitizedData *> *getDigitizedData() const { return digitizedData; }

private:

	// index is hit number
	vector<GTrueInfoData *> *trueInfosData = nullptr;
	vector<GDigitizedData *> *digitizedData = nullptr;

	GLogger * const log;

};


#endif
