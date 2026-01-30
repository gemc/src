/**
 * \file GDigitizedData.cc
 * \brief Implementation of the GDigitizedData class.
 *
 * This file implements storage, filtering, and accumulation of digitized observables.
 * "Digitized" means quantities produced after detector response + electronics logic
 * (ADC/TDC-like values, integrated charge, derived timing, and readout coordinates).
 */

#include "gDigitizedData.h"
#include "gdataConventions.h"

// c++
#include <string>
#include <map>
#include <vector>

#include "gdynamicDigitization/gdynamicdigitization_options.h"

/// Global counter used only by the static factory \ref GDigitizedData::create().
/// It is intentionally thread-safe to support examples and tests running in parallel.
std::atomic<int> GDigitizedData::globalDigitizedDataCounter{0};

GDigitizedData::GDigitizedData(const std::shared_ptr<GOptions>& gopts, const GHit* ghit)
	: GBase(gopts, GDIGITIZED_DATA_LOGGER) {
	// Copy hit identity (sector/layer/component...) from the originating hit.
	gidentity = ghit->getGID();
}

std::map<std::string, int> GDigitizedData::getIntObservablesMap(int which) const {
	// Filtering is based on whether the key is a streaming readout key (SRO) or not.
	// which=0 -> return non-SRO variables (physics content)
	// which=1 -> return only SRO variables (crate/slot/channel/time/charge)
	std::map<std::string, int> filteredIntObservablesMap;
	for (const auto& [varName, value] : intObservablesMap) {
		if (validVarName(varName, which)) { filteredIntObservablesMap[varName] = value; }
	}
	log->info(2, " getting ", which, " from intObservablesMap.");
	return filteredIntObservablesMap;
}

std::map<std::string, double> GDigitizedData::getDblObservablesMap(int which) const {
	// Same filtering semantics as getIntObservablesMap().
	std::map<std::string, double> filteredDblObservablesMap;
	for (const auto& [varName, value] : doubleObservablesMap) {
		if (validVarName(varName, which)) { filteredDblObservablesMap[varName] = value; }
	}
	log->info(2, " getting ", which, " from doubleObservablesMap.");
	return filteredDblObservablesMap;
}

// SRO:
bool GDigitizedData::validVarName(const std::string& varName, int which) {
	// "SRO variables" encode the streaming readout coordinates/time/charge and are
	// handled separately by some output backends.
	bool isSROVar = (varName == CRATESTRINGID || varName == SLOTSTRINGID || varName == CHANNELSTRINGID ||
	                 varName == CHARGEATELECTRONICS || varName == TIMEATELECTRONICS);
	if (which == 0) {
		if (isSROVar) { return false; }
	}
	else if (which == 1) {
		if (!isSROVar) { return false; }
	}
	return true;
}

void GDigitizedData::includeVariable(const std::string& vname, int value) {
	// Store/overwrite a per-hit integer observable.
	// Typical use: indices (component, paddle, hit number) or integerized electronics values.
	log->info(2, "Including int variable ", vname, " with value ", value);
	intObservablesMap[vname] = value;
}

void GDigitizedData::includeVariable(const std::string& vname, double value) {
	// Store/overwrite a per-hit floating observable.
	// Typical use: ADC-like floating values, calibrated energies, or continuous timing.
	log->info(2, "Including double variable ", vname, " with value ", value);
	doubleObservablesMap[vname] = value;
}

void GDigitizedData::accumulateVariable(const std::string& vname, int value) {
	// Accumulate is meant for run-level integration: sum contributions into the same key.
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
	// Accumulate is meant for run-level integration: sum contributions into the same key.
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
	// Convenience accessor for TIMEATELECTRONICS (if present).
	// If absent, return a sentinel to make missing-data obvious to consumers.
	if (intObservablesMap.find(TIMEATELECTRONICS) == intObservablesMap.end()) { return TIMEATELECTRONICSNOTDEFINED; }
	log->info(2, "Getting TIMEATELECTRONICS from intObservablesMap.");
	return intObservablesMap[TIMEATELECTRONICS];
}

int GDigitizedData::getIntObservable(const std::string& varName) {
	// Access one integer observable by name.
	// If missing, emit an error message via the logger (typically fatal depending on configuration).
	if (intObservablesMap.find(varName) == intObservablesMap.end()) {
		log->error(ERR_VARIABLENOTFOUND,
		           "variable name <" + varName + "> not found in GDigitizedData::intObservablesMap");
	}
	return intObservablesMap[varName];
}

double GDigitizedData::getDblObservable(const std::string& varName) {
	// Access one double observable by name.
	if (doubleObservablesMap.find(varName) == doubleObservablesMap.end()) {
		log->error(ERR_VARIABLENOTFOUND,
		           "variable name <" + varName + "> not found in GDigitizedData::doubleObservablesMap");
	}
	return doubleObservablesMap[varName];
}

std::string GDigitizedData::getIdentityString() const {
	// Produces a concise representation:
	//   name1->value1, name2->value2, ...
	// This is useful for debugging and for labeling output records.
	std::string identifierString;
	for (size_t i = 0; i < gidentity.size() - 1; i++) {
		identifierString += gidentity[i].getName() + "->" + std::to_string(gidentity[i].getValue()) + ", ";
	}
	identifierString += gidentity.back().getName() + "->" + std::to_string(gidentity.back().getValue());
	return identifierString;
}
