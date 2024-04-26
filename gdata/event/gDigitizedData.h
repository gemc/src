#ifndef GDIGITIZEDHIT_H
#define GDIGITIZEDHIT_H 1


// observable types include int, float, double,
#include <string>
#include <map>
#include <vector>
using std::string;
using std::map;
using std::vector;

// glibrary
#include "ghit.h"

// in each hit, the digitization can produce:
// - single values
// - vectors of values
// requirement: the variable names for each type must be unique
class GDigitizedData {
	
public:

	GDigitizedData(GHit *ghit);
	vector<GIdentifier> getIdentity() const {return gidentity;}

	// public interface to add data to a hit
	void includeVariable(string vname, int value);
	void includeVariable(string vname, float value);

	void includeVariable(string vname, vector<int> values);
	void includeVariable(string vname, vector<float> values);

	// which = 0: only returns non SRO vars
	// which = 1: returns all vars
	const map<string, int>    getIntObservablesMap(int which) const;
	const map<string, float>  getFltObservablesMap(int which) const;

	// returns -1 if TIMEATELECTRONICS is not added to the digitization
	int getTimeAtElectronics();

	int getIntObservable(string varName);
	float getFltObservable(string varName);

	inline const map<string, vector<int>>   getArrayIntObservablesMap() const {return arrayIntObservablesMap;}
	inline const map<string, vector<float>> getArrayFltObservablesMap() const {return arrayFltObservablesMap;}


private:

	// the data map are keyd with the variable name
	map<string, int>    intObservablesMap;
	map<string, float>  fltObservablesMap;

	map<string, vector<int> >   arrayIntObservablesMap;
	map<string, vector<float> > arrayFltObservablesMap;

	// set at constructor, from the hit
	vector<GIdentifier> gidentity;

	bool validVarName(string varName, int which) const;

};


#endif
