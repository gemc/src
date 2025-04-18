/**
 * \file GDigitizedData.cc
 * \brief Implementation of the GDigitizedData class.
 */

#include "gDigitizedData.h"
#include "../gdataConventions.h"
#include <string>
#include <map>
#include <vector>

GDigitizedData::GDigitizedData(GHit *ghit, const std::shared_ptr<GLogger>& logger) : log(std::move(logger)) {
	log->debug(CONSTRUCTOR, "GDigitizedData");
	gidentity = ghit->getGID();
}

std::map<std::string, int> GDigitizedData::getIntObservablesMap(int which) const {
	std::map<std::string, int> filteredIntObservablesMap;
	for (const auto& [varName, value] : intObservablesMap) {
		if (validVarName(varName, which)) {
			filteredIntObservablesMap[varName] = value;
		}
	}
	log->debug(NORMAL, " getting ", which, " from intObservablesMap.");
	return filteredIntObservablesMap;
}

std::map<std::string, float> GDigitizedData::getFltObservablesMap(int which) const {
	std::map<std::string, float> filteredFltObservablesMap;
	for (const auto& [varName, value] : doubleObservablesMap) {
		if (validVarName(varName, which)) {
			filteredFltObservablesMap[varName] = value;
		}
	}
	log->debug(NORMAL, " getting ", which, " from doubleObservablesMap.");
	return filteredFltObservablesMap;
}

bool GDigitizedData::validVarName(const std::string& varName, int which) const {
	bool isSROVar = (varName == CRATESTRINGID || varName == SLOTSTRINGID || varName == CHANNELSTRINGID ||
					 varName == CHARGEATELECTRONICS || varName == TIMEATELECTRONICS);
	if (which == 0) {
		if (isSROVar) {
			return false;
		}
	} else if (which == 1) {
		if (!isSROVar) {
			return false;
		}
	}
	return true;
}

void GDigitizedData::includeVariable(const std::string& vname, int value) {
	log->debug(NORMAL, "Including int variable ", vname, " with value ", value);
	intObservablesMap[vname] = value;
}

void GDigitizedData::includeVariable(const std::string& vname, double value) {
	log->debug(NORMAL, "Including float variable ", vname, " with value ", value);
	doubleObservablesMap[vname] = value;
}

int GDigitizedData::getTimeAtElectronics() {
	if (intObservablesMap.find(TIMEATELECTRONICS) == intObservablesMap.end()) {
		return TIMEATELECTRONICSNOTDEFINED;
	}
	log->debug(NORMAL, "Getting TIMEATELECTRONICS from intObservablesMap.");
	return intObservablesMap[TIMEATELECTRONICS];
}

int GDigitizedData::getIntObservable(std::string varName) {
	if (intObservablesMap.find(varName) == intObservablesMap.end()) {
		log->error(EC__VARIABLENOTFOUND, "variable name <" + varName + "> not found in GDigitizedData::intObservablesMap");
	}
	return intObservablesMap[varName];
}

float GDigitizedData::getFltObservable(std::string varName) {
	if (doubleObservablesMap.find(varName) == doubleObservablesMap.end()) {
		log->error(EC__VARIABLENOTFOUND, "variable name <" + varName + "> not found in GDigitizedData::doubleObservablesMap");
	}
	return doubleObservablesMap[varName];
}

std::string GDigitizedData::getIdentityString() {
	std::string identifierString = "";
	for (size_t i = 0; i < gidentity.size() - 1; i++) {
		identifierString += gidentity[i].getName() + "->" + std::to_string(gidentity[i].getValue()) + ", ";
	}
	identifierString += gidentity.back().getName() + "->" + std::to_string(gidentity.back().getValue());
	return identifierString;
}
