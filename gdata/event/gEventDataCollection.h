#ifndef  GEVENTDATA_H
#define  GEVENTDATA_H  1

// gdata
#include "gEventDataCollectionHeader.h"
#include "../gdataConventions.h"
#include "gDataCollection.h"

// c++
#include <vector>

class GEventDataCollection {
public:
	// notice that the logger must come here with the 'gdata' name in the constructor
	GEventDataCollection(GEventDataCollectionHeader *header, GLogger *logger) : gheader(header), log(logger) {
		log->debug(CONSTRUCTOR, "GEventDataCollection");
		gdataCollectionMap = new map<string, GDataCollection *>();
	}

	~GEventDataCollection() {

		log->debug(DESTRUCTOR, "GEventDataCollection");

		// PRAGMA TODO: what do delete here?
		for (auto &[keys, values]: (*gdataCollectionMap)) {
			delete values;
		}
		delete gheader;
		delete gdataCollectionMap;
		delete log;
	}

public:
	// api to add data
	void addDetectorTrueInfoData(string sdName, GTrueInfoData *data);

	void addDetectorDigitizedData(string sdName, GDigitizedData *data);

	// getters
	// we want to crash if the pointers do not exist
	inline const GEventDataCollectionHeader *getHeader() const { return gheader; }

	inline const map<string, GDataCollection *> *getDataCollectionMap() const { return gdataCollectionMap; }

	inline int getEventNumber() const { return gheader->getG4LocalEvn(); }

	const vector<GTrueInfoData *> *getTrueInfoDataForDetector(string detector) const;

	const vector<GDigitizedData *> *getDigitizedDataForDetector(string detector) const;

private:

	GLogger *log;
	GEventDataCollectionHeader *gheader = nullptr;

	// key is sensitive detector name
	// each GDataCollection is a vector, indexed by hit number
	map<string, GDataCollection *> *gdataCollectionMap;


};


#endif
