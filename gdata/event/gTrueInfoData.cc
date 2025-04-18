/**
 * \file gTrueInfoData.cc
 * \brief Implementation of the GTrueInfoData class.
 */

#include "gTrueInfoData.h"
#include <string>
#include <utility>

GTrueInfoData::GTrueInfoData(GHit* ghit, std::shared_ptr<GLogger> logger) : log(std::move(logger)) {
	log->debug(CONSTRUCTOR, "GTrueInfoData");
	gidentity = ghit->getGID();
}

void GTrueInfoData::includeVariable(const std::string& varName, float var) {
	log->debug(NORMAL, " including ", varName, " in trueInfoDoublesVariablesMap.");
	trueInfoDoublesVariablesMap[varName] = var;
}

void GTrueInfoData::includeVariable(const std::string& varName, std::string var) {
	log->debug(NORMAL, " including ", varName, " in trueInfoStringVariablesMap.");
	trueInfoStringVariablesMap[varName] = std::move(var);
}

std::string GTrueInfoData::getIdentityString() {
	std::string identifierString;
	for (size_t i = 0; i < gidentity.size() - 1; i++) {
		identifierString += gidentity[i].getName() + "->" + std::to_string(gidentity[i].getValue()) + ", ";
	}
	identifierString += gidentity.back().getName() + "->" + std::to_string(gidentity.back().getValue());
	return identifierString;
}
