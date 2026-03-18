/**
 * \file gTrueInfoData.cc
 * \brief Implementation of GTrueInfoData.
 *
 * \details
 * The header gTrueInfoData.h contains the authoritative API documentation, usage semantics,
 * and data-model notes for this class.
 */

#include "gTrueInfoData.h"
#include <string>
#include <utility>

// Thread-safe counter used only by the example/test factory create().
std::atomic<int> GTrueInfoData::globalTrueInfoDataCounter{0};

GTrueInfoData::GTrueInfoData(const std::shared_ptr<GOptions>& gopts, const GHit* ghit)
	: GBase(gopts, GTRUEDATA_LOGGER) {
	// Copy the hit identity so this object remains self-contained after the source hit expires.
	gidentity = ghit->getGID();
}

void GTrueInfoData::includeVariable(const std::string& varName, double value) {
	// Event-level insertion with overwrite semantics.
	doubleObservablesMap[varName] = value;
	log->info(2, FUNCTION_NAME, " including ", varName, " in trueInfoDoublesVariablesMap with value: ", value);
}

void GTrueInfoData::includeVariable(const std::string& varName, std::string value) {
	// Event-level insertion with overwrite semantics.
	log->info(2, FUNCTION_NAME, " including ", varName, " in trueInfoStringVariablesMap  with value:", value);
	stringVariablesMap[varName] = std::move(value);
}

void GTrueInfoData::accumulateVariable(const std::string& vname, double value) {
	// Run/integrated accumulation by summation.
	if (doubleObservablesMap.find(vname) == doubleObservablesMap.end()) {
		doubleObservablesMap[vname] = value;
		log->info(2, FUNCTION_NAME, "Creating double variable ", vname, " with value ", value, ", sum is now:",
		          doubleObservablesMap[vname]);
	}
	else {
		doubleObservablesMap[vname] += value;
		log->info(2, FUNCTION_NAME, "Accumulating double variable ", vname, " with value ", value, ", sum is now:",
		          doubleObservablesMap[vname]);
	}
}

std::string GTrueInfoData::getIdentityString() const {
	// Build a compact label from the stored identifier vector.
	std::string identifierString;
	for (size_t i = 0; i < gidentity.size() - 1; i++) {
		identifierString += gidentity[i].getName() + "->" + std::to_string(gidentity[i].getValue()) + ", ";
	}
	identifierString += gidentity.back().getName() + "->" + std::to_string(gidentity.back().getValue());
	return identifierString;
}

std::ostream& operator<<(std::ostream& os, const GTrueInfoData& data) {
	os << "GTrueInfoData{identity=\"" << data.getIdentityString() << "\"";

	if (!data.doubleObservablesMap.empty()) {
		os << ", doubleObservables={";
		bool first = true;
		for (const auto& [name, value] : data.doubleObservablesMap) {
			if (!first) {
				os << ", ";
			}
			os << name << ": " << value;
			first = false;
		}
		os << "}";
	}

	if (!data.stringVariablesMap.empty()) {
		os << ", stringObservables={";
		bool first = true;
		for (const auto& [name, value] : data.stringVariablesMap) {
			if (!first) {
				os << ", ";
			}
			os << name << ": \"" << value << "\"";
			first = false;
		}
		os << "}";
	}

	os << "}";
	return os;
}