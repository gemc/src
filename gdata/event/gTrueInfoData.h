#ifndef GTRUEINFOHIT_H
#define GTRUEINFOHIT_H 1

#include <string>
#include <map>
#include <vector>

// gemc
#include "ghit.h"
#include "glogger.h"

// data for a single
class GTrueInfoData {
	
public:
	GTrueInfoData(GHit *ghit, GLogger * const logger);

	~GTrueInfoData() {
		log->debug(DESTRUCTOR, "GTrueInfoData");
	}

	string getIdentityString();

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

	GLogger * const log;

};


#endif
