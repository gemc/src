/**
 * \file GDigitizedData.cc
 * \brief Implementation of the GDigitizedData class.
 */

#include "gDigitizedData.h"
#include "gdataConventions.h"

// c++
#include <string>
#include <map>
#include <vector>

std::atomic<int> GDigitizedData::globalDigitizedDataCounter{0};

GDigitizedData::GDigitizedData(const std::unique_ptr<GHit>& ghit, const std::shared_ptr<GLogger>& logger) : log(logger) {
	log->debug(CONSTRUCTOR, "GDigitizedData");
	gidentity = ghit->getGID();
}

std::map<std::string, int> GDigitizedData::getIntObservablesMap(int which) const {
	std::map<std::string, int> filteredIntObservablesMap;
	for (const auto& [varName, value] : intObservablesMap) { if (validVarName(varName, which)) { filteredIntObservablesMap[varName] = value; } }
	log->info(2, " getting ", which, " from intObservablesMap.");
	return filteredIntObservablesMap;
}

std::map<std::string, double> GDigitizedData::getDblObservablesMap(int which) const {
	std::map<std::string, double> filteredDblObservablesMap;
	for (const auto& [varName, value] : doubleObservablesMap) { if (validVarName(varName, which)) { filteredDblObservablesMap[varName] = value; } }
	log->info(2, " getting ", which, " from doubleObservablesMap.");
	return filteredDblObservablesMap;
}

bool GDigitizedData::validVarName(const std::string& varName, int which) {
	bool isSROVar = (varName == CRATESTRINGID || varName == SLOTSTRINGID || varName == CHANNELSTRINGID ||
	                 varName == CHARGEATELECTRONICS || varName == TIMEATELECTRONICS);
	if (which == 0) { if (isSROVar) { return false; } }
	else if (which == 1) { if (!isSROVar) { return false; } }
	return true;
}

void GDigitizedData::includeVariable(const std::string& vname, int value) {
	log->info(2, "Including int variable ", vname, " with value ", value);
	intObservablesMap[vname] = value;
}

void GDigitizedData::includeVariable(const std::string& vname, double value) {
	log->info(2, "double variable ", vname, " with value ", value);
	doubleObservablesMap[vname] = value;
}

int GDigitizedData::getTimeAtElectronics() {
	if (intObservablesMap.find(TIMEATELECTRONICS) == intObservablesMap.end()) { return TIMEATELECTRONICSNOTDEFINED; }
	log->info(2, "Getting TIMEATELECTRONICS from intObservablesMap.");
	return intObservablesMap[TIMEATELECTRONICS];
}

int GDigitizedData::getIntObservable(const std::string& varName) {
	if (intObservablesMap.find(varName) == intObservablesMap.end()) {
		log->error(ERR_VARIABLENOTFOUND, "variable name <" + varName + "> not found in GDigitizedData::intObservablesMap");
	}
	return intObservablesMap[varName];
}

double GDigitizedData::getDblObservable(const std::string& varName) {
	if (doubleObservablesMap.find(varName) == doubleObservablesMap.end()) {
		log->error(ERR_VARIABLENOTFOUND, "variable name <" + varName + "> not found in GDigitizedData::doubleObservablesMap");
	}
	return doubleObservablesMap[varName];
}

std::string GDigitizedData::getIdentityString() {
	std::string identifierString;
	for (size_t i = 0; i < gidentity.size() - 1; i++) { identifierString += gidentity[i].getName() + "->" + std::to_string(gidentity[i].getValue()) + ", "; }
	identifierString += gidentity.back().getName() + "->" + std::to_string(gidentity.back().getValue());
	return identifierString;
}
