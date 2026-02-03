/**
 * \file GDigitizedData.cc
 * \brief Implementation of \ref GDigitizedData.
 *
 * \details
 * See \ref GDigitizedData.h for the authoritative API documentation, including SRO filtering
 * semantics and event-vs-run usage patterns.
 */

// See header for API docs.

#include "gDigitizedData.h"
#include "gdataConventions.h"

// c++
#include <string>
#include <map>
#include <vector>

#include "gdynamicDigitization/gdynamicdigitization_options.h"

/// Global counter used only by the static factory \ref GDigitizedData::create "create()".
/// It is intentionally thread-safe to support examples and tests running in parallel.
///
/// \details
/// This counter is not intended for physics production; it exists for deterministic example data.
std::atomic<int> GDigitizedData::globalDigitizedDataCounter{0};

GDigitizedData::GDigitizedData(const std::shared_ptr<GOptions>& gopts, const GHit* ghit)
	: GBase(gopts, GDIGITIZED_DATA_LOGGER) {
	// Copy hit identity (sector/layer/component...) from the originating hit.
	//
	// Ownership and lifetime:
	// - ghit is not owned.
	// - gidentity is copied so the digitized object is self-contained and remains valid
	//   even after the originating hit is destroyed.
	gidentity = ghit->getGID();
}

std::map<std::string, int> GDigitizedData::getIntObservablesMap(int which) const {
	// Return a filtered copy of the integer observables.
	//
	// which=0 -> return non-SRO variables (digitization outputs / physics-like quantities)
	// which=1 -> return only SRO variables (crate/slot/channel/timeAtElectronics/chargeAtElectronics)
	std::map<std::string, int> filteredIntObservablesMap;
	for (const auto& [varName, value] : intObservablesMap) {
		if (validVarName(varName, which)) { filteredIntObservablesMap[varName] = value; }
	}
	log->info(2, " getting ", which, " from intObservablesMap.");
	return filteredIntObservablesMap;
}

std::map<std::string, double> GDigitizedData::getDblObservablesMap(int which) const {
	// Return a filtered copy of the double observables.
	//
	// Uses the same filtering semantics as getIntObservablesMap().
	std::map<std::string, double> filteredDblObservablesMap;
	for (const auto& [varName, value] : doubleObservablesMap) {
		if (validVarName(varName, which)) { filteredDblObservablesMap[varName] = value; }
	}
	log->info(2, " getting ", which, " from doubleObservablesMap.");
	return filteredDblObservablesMap;
}

// SRO:
bool GDigitizedData::validVarName(const std::string& varName, int which) {
	// Decide whether \p varName should be included in a filtered view.
	//
	// A variable is considered SRO if it matches one of the conventional keys defined in
	// gdataConventions.h.
	bool isSROVar = (varName == CRATESTRINGID || varName == SLOTSTRINGID || varName == CHANNELSTRINGID ||
		varName == CHARGEATELECTRONICS || varName == TIMEATELECTRONICS);

	if (which == 0) {
		// Return only non-SRO keys.
		if (isSROVar) { return false; }
	}
	else if (which == 1) {
		// Return only SRO keys.
		if (!isSROVar) { return false; }
	}

	return true;
}

void GDigitizedData::includeVariable(const std::string& vname, int value) {
	// Store/overwrite a per-hit integer observable (event-level).
	log->info(2, "Including int variable ", vname, " with value ", value);
	intObservablesMap[vname] = value;
}

void GDigitizedData::includeVariable(const std::string& vname, double value) {
	// Store/overwrite a per-hit floating observable (event-level).
	log->info(2, "Including double variable ", vname, " with value ", value);
	doubleObservablesMap[vname] = value;
}

void GDigitizedData::accumulateVariable(const std::string& vname, int value) {
	// Accumulate an integer observable for run-level integration (summation).
	if (intObservablesMap.find(vname) == intObservablesMap.end()) {
		log->info(2, "Accumulating new int variable ", vname, " with value ", value);
		intObservablesMap[vname] = value;
	}
	else {
		log->info(2, "Accumulating int variable ", vname, " with value ", value);
		intObservablesMap[vname] += value;
	}
}

void GDigitizedData::accumulateVariable(const std::string& vname, double value) {
	// Accumulate a double observable for run-level integration (summation).
	if (doubleObservablesMap.find(vname) == doubleObservablesMap.end()) {
		log->info(2, "Accumulating double variable ", vname, " with value ", value);
		doubleObservablesMap[vname] = value;
	}
	else {
		log->info(2, "Accumulating double variable ", vname, " with value ", value);
		doubleObservablesMap[vname] += value;
	}
}

int GDigitizedData::getTimeAtElectronics() {
	// Convenience accessor for TIMEATELECTRONICS.
	if (intObservablesMap.find(TIMEATELECTRONICS) == intObservablesMap.end()) { return TIMEATELECTRONICSNOTDEFINED; }
	log->info(2, "Getting TIMEATELECTRONICS from intObservablesMap.");
	return intObservablesMap[TIMEATELECTRONICS];
}

int GDigitizedData::getIntObservable(const std::string& varName) {
	// Retrieve one integer observable by name.
	if (intObservablesMap.find(varName) == intObservablesMap.end()) {
		log->error(ERR_VARIABLENOTFOUND,
		           "variable name <" + varName + "> not found in GDigitizedData::intObservablesMap");
	}
	return intObservablesMap[varName];
}

double GDigitizedData::getDblObservable(const std::string& varName) {
	// Retrieve one double observable by name.
	if (doubleObservablesMap.find(varName) == doubleObservablesMap.end()) {
		log->error(ERR_VARIABLENOTFOUND,
		           "variable name <" + varName + "> not found in GDigitizedData::doubleObservablesMap");
	}
	return doubleObservablesMap[varName];
}

std::string GDigitizedData::getIdentityString() const {
	// Build a concise identity label from the \c gidentity vector.
	std::string identifierString;
	for (size_t i = 0; i < gidentity.size() - 1; i++) {
		identifierString += gidentity[i].getName() + "->" + std::to_string(gidentity[i].getValue()) + ", ";
	}
	identifierString += gidentity.back().getName() + "->" + std::to_string(gidentity.back().getValue());
	return identifierString;
}
