/**
 * \file gTrueInfoData.cpp
 * \brief Implementation of the GTrueInfoData class.
 */

#include "gTrueInfoData.h"
#include <string>

GTrueInfoData::GTrueInfoData(GHit *ghit, GLogger * const logger) : log(logger) {
	log->debug(CONSTRUCTOR, "GTrueInfoData");
	gidentity = ghit->getGID();
}

void GTrueInfoData::includeVariable(std::string varName, float var) {
	log->debug(NORMAL, " including ", varName, " in trueInfoFloatVariablesMap.");
	trueInfoFloatVariablesMap[varName] = var;
}

void GTrueInfoData::includeVariable(std::string varName, std::string var) {
	log->debug(NORMAL, " including ", varName, " in trueInfoStringVariablesMap.");
	trueInfoStringVariablesMap[varName] = var;
}

std::string GTrueInfoData::getIdentityString() {
	std::string identifierString = "";
	for (size_t i = 0; i < gidentity.size() - 1; i++) {
		identifierString += gidentity[i].getName() + "->" + std::to_string(gidentity[i].getValue()) + ", ";
	}
	identifierString += gidentity.back().getName() + "->" + std::to_string(gidentity.back().getValue());
	return identifierString;
}
