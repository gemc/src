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
	log->info(2, " including ", varName, " in trueInfoDoublesVariablesMap.");
	doubleObservablesMap[varName] = value;
}

void GTrueInfoData::includeVariable(const std::string& varName, std::string value) {
	log->info(2, " including ", varName, " in trueInfoStringVariablesMap.");
	stringVariablesMap[varName] = std::move(value);
}


void GTrueInfoData::accumulateVariable(const std::string& vname, std::string value) {
	if (stringVariablesMap.find(vname) == stringVariablesMap.end()) {
		log->info(2, "Accumulating new int variable ", vname, " with value ", value);
		stringVariablesMap[vname] = value;
	}
	else {
		log->info(2, "Accumulating int variable ", vname, " with value ", value);
		stringVariablesMap[vname] += value;
	}
}
void GTrueInfoData::accumulateVariable(const std::string& vname, double value) {
	if (doubleObservablesMap.find(vname) == doubleObservablesMap.end()) {
		log->info(2, "Accumulating double variable ", vname, " with value ", value);
		doubleObservablesMap[vname] = value;
	}
	else {
		log->info(2, "Accumulating double variable ", vname, " with value ", value);
		doubleObservablesMap[vname] += value;
	}
}



std::string GTrueInfoData::getIdentityString() const  {
	std::string identifierString;
	for (size_t i = 0; i < gidentity.size() - 1; i++) { identifierString += gidentity[i].getName() + "->" + std::to_string(gidentity[i].getValue()) + ", "; }
	identifierString += gidentity.back().getName() + "->" + std::to_string(gidentity.back().getValue());
	return identifierString;
}


