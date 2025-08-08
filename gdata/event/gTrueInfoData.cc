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

void GTrueInfoData::includeVariable(const std::string& varName, double var) {
	log->info(2, " including ", varName, " in trueInfoDoublesVariablesMap.");
	trueInfoDoublesVariablesMap[varName] = var;
}

void GTrueInfoData::includeVariable(const std::string& varName, std::string var) {
	log->info(2, " including ", varName, " in trueInfoStringVariablesMap.");
	trueInfoStringVariablesMap[varName] = std::move(var);
}

std::string GTrueInfoData::getIdentityString() const  {
	std::string identifierString;
	for (size_t i = 0; i < gidentity.size() - 1; i++) { identifierString += gidentity[i].getName() + "->" + std::to_string(gidentity[i].getValue()) + ", "; }
	identifierString += gidentity.back().getName() + "->" + std::to_string(gidentity.back().getValue());
	return identifierString;
}
