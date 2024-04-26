// gdata
#include "gDigitizedData.h"
#include "../gdataConventions.h"

// gutilities for the conventions and gexit
#include "gutilities.h"

// c++
using namespace std;

// the methods below filter out the streaming vars
map<string, int> const GDigitizedData::getIntObservablesMap(int which) const {
	map<string, int> filteredIntObservablesMap;

	for ( auto [varName, value]: intObservablesMap) {
		if ( validVarName(varName, which) ) {
			filteredIntObservablesMap[varName] = value;
		}
	}
	return filteredIntObservablesMap;
}

map<string, float> const GDigitizedData::getFltObservablesMap(int which) const {
	map<string, float> filteredFltObservablesMap;


	for ( auto [varName, value]: fltObservablesMap) {
		if ( validVarName(varName, which) ) {
			filteredFltObservablesMap[varName] = value;
		}
	}

	return filteredFltObservablesMap;
}


// criteria for valid var name: it's not any of the streaming var
// which = 0: only returns non SRO vars
// which = 1: returns all vars
bool GDigitizedData::validVarName(string varName, int which) const {

	bool isSROVar = (varName == CRATESTRINGID || varName == SLOTSTRINGID || varName == CHANNELSTRINGID || varName == CHARGEATELECTRONICS || varName == TIMEATELECTRONICS);

	if ( which == 0 ) {
		if ( isSROVar ) {
			return false;
		}
	} else if ( which == 1 ) {
		if ( !isSROVar ) {
			return false;
		}
	}
	return true;
}

GDigitizedData::GDigitizedData(GHit *ghit) {
	gidentity = ghit->getGID();
}

void GDigitizedData::includeVariable(string vname, int value) {
	intObservablesMap[vname] = value;
}

void GDigitizedData::includeVariable(string vname, float value) {
	fltObservablesMap[vname] = value;
}

// returns -1 if TIMEATELECTRONICS is not added to the digitization
int GDigitizedData::getTimeAtElectronics() {
	
	if ( intObservablesMap.find(TIMEATELECTRONICS) == intObservablesMap.end() ) {
		return TIMEATELECTRONICSNOTDEFINED;
	}
	return intObservablesMap[TIMEATELECTRONICS];
}


int GDigitizedData::getIntObservable(string varName) {

	if ( intObservablesMap.find(varName) == intObservablesMap.end() ) {
		cerr << FATALERRORL << "variable name <" << varName << "> not found in GDigitizedData::intObservablesMap" << endl;
		gexit(EC__VARIABLENOTFOUND);
	}
	return intObservablesMap[varName];
}

float GDigitizedData::getFltObservable(string varName) {
	if ( fltObservablesMap.find(varName) == fltObservablesMap.end() ) {
		cerr << FATALERRORL << "variable name <" << varName << "> not found in GDigitizedData::fltObservablesMap" << endl;
		gexit(EC__VARIABLENOTFOUND);
	}
	return fltObservablesMap[varName];

}
