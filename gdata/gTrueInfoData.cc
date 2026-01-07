/**
 * \file gTrueInfoData.cc
 * \brief Implementation of the GTrueInfoData class.
 */

#include "gTrueInfoData.h"
#include <string>
#include <utility>

std::atomic<int> GTrueInfoData::globalTrueInfoDataCounter{0};

GTrueInfoData::GTrueInfoData(const std::shared_ptr<GOptions>& gopts, const GHit* ghit) : GBase(gopts, GTRUEDATA_LOGGER) {
	gidentity = ghit->getGID();
}

void GTrueInfoData::includeVariable(const std::string& varName, double value) {
	doubleObservablesMap[varName] = value;
	log->info(2, FUNCTION_NAME, " including ", varName, " in trueInfoDoublesVariablesMap with value: ", value);
}

void GTrueInfoData::includeVariable(const std::string& varName, std::string value) {
	log->info(2, FUNCTION_NAME, " including ", varName, " in trueInfoStringVariablesMap  with value:", value);
	stringVariablesMap[varName] = std::move(value);
}


void GTrueInfoData::accumulateVariable(const std::string& vname, double value) {
	if (doubleObservablesMap.find(vname) == doubleObservablesMap.end()) {
		doubleObservablesMap[vname] = value;
		log->info(2, FUNCTION_NAME, "Creating double variable ", vname, " with value ", value, ", sum is now:", doubleObservablesMap[vname]);
	}
	else {
		doubleObservablesMap[vname] += value;
		log->info(2, FUNCTION_NAME, "Accumulating double variable ", vname, " with value ", value, ", sum is now:", doubleObservablesMap[vname]);
	}
}



std::string GTrueInfoData::getIdentityString() const  {
	std::string identifierString;
	for (size_t i = 0; i < gidentity.size() - 1; i++) { identifierString += gidentity[i].getName() + "->" + std::to_string(gidentity[i].getValue()) + ", "; }
	identifierString += gidentity.back().getName() + "->" + std::to_string(gidentity.back().getValue());
	return identifierString;
}


