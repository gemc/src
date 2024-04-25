#ifndef  GEVENTDATA_H
#define  GEVENTDATA_H  1

// gdata
#include "gEventDataCollectionHeader.h"
#include "../gdataConventions.h"
#include "gDataCollection.h"

// c++
#include <vector>
using std::vector;

class GEventDataCollection
{
public:
	// construct event data using a GEventHeader
	GEventDataCollection(GEventDataCollectionHeader* header, int v = 0 ) : verbosity(v), gheader(header) {
		if ( verbosity >= GVERBOSITY_CLASSES) {
			gLogClassConstruct("GEventDataCollection");
		}
		gdataCollectionMap = new map<string, GDataCollection*>();
	}

	~GEventDataCollection() {

		if ( verbosity >= GVERBOSITY_CLASSES) {
			gLogClassDestruct("GEventDataCollection");
		}

		// PRAGMA TODO: what do delete here?
		for (auto& [keys, values]: (*gdataCollectionMap) ) {
			delete values;
		}
		delete gheader;
		delete gdataCollectionMap;
	}

public:
	// api to add data
	void addDetectorTrueInfoData(string sdName,  GTrueInfoData *data);
	void addDetectorDigitizedData(string sdName, GDigitizedData *data);

	// getters
	// we want to crash if the pointers do not exist
	inline const GEventDataCollectionHeader*  getHeader()               const { return gheader; }
	inline const map<string, GDataCollection*>*  getDataCollectionMap() const { return gdataCollectionMap; }
	inline int getEventNumber()                                   const { return gheader->getG4LocalEvn(); }

	const vector<GTrueInfoData*>  *getTrueInfoDataForDetector(string detector) const;
	const vector<GDigitizedData*> *getDigitizedDataForDetector(string detector) const;

private:
	int verbosity;

	GEventDataCollectionHeader *gheader = nullptr;

	// key is sensitive detector name
	// each GDataCollection is a vector, indexed by hit number
	map<string, GDataCollection*> *gdataCollectionMap;


};


#endif
