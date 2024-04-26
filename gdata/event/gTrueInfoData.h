#ifndef GTRUEINFOHIT_H
#define GTRUEINFOHIT_H 1

#include <string>
#include <map>
#include <vector>
using std::string;
using std::map;
using std::vector;

// glibrary
#include "ghit.h"

// data for a single bu
class GTrueInfoData {
	
public:
	GTrueInfoData(GHit *ghit);
	vector<GIdentifier> getIdentity() const {return gidentity;}

	void includeVariable(string varName, float var);
	void includeVariable(string varName, string var);

	inline const map<string, float> getFloatVariablesMap()   const {return trueInfoFloatVariablesMap;}

	// stores process name
	inline const map<string, string> getStringVariablesMap() const {return trueInfoStringVariablesMap;}

private:

	// the data map are keyd with the variable name
	map<string, float>  trueInfoFloatVariablesMap ;
	map<string, string> trueInfoStringVariablesMap ;

	vector<GIdentifier> gidentity;

};


#endif
