#ifndef GHITSCOLLECTION_H
#define GHITSCOLLECTION_H 1

// gdata
#include "gTrueInfoData.h"
#include "gDigitizedData.h"

// observable types include int, float, double,
#include <string>
#include <map>
#include <vector>
using std::string;
using std::map;
using std::vector;


// collected for each sensitive detector
class GDataCollection {

public:
	GDataCollection() {
		trueInfosData  = new vector<GTrueInfoData*>;
		digitizedData  = new vector<GDigitizedData*>;
	}
	
	~GDataCollection() {

		for ( auto* hit: (*trueInfosData) )  { delete hit; }
		for ( auto* hit: (*digitizedData) )  { delete hit; }

		delete trueInfosData;
		delete digitizedData;
	}


public:

	// public interface to add hit
	void addTrueInfoData( GTrueInfoData *data);
	void addDigitizedData(GDigitizedData *data);

	// one entry / hit
	inline const vector<GTrueInfoData*>  *getTrueInfoData()  const { return trueInfosData;}
	inline const vector<GDigitizedData*> *getDigitizedData() const { return digitizedData;}

private:

	// index is hit number
	vector<GTrueInfoData*>  *trueInfosData = nullptr;
	vector<GDigitizedData*> *digitizedData = nullptr;

};



#endif
