/**
 * \file GDigitizedData.cc
 * \brief Implementation of the \ref GDigitizedData class.
 *
 * \details
 * This file implements storage, filtering, and accumulation of digitized observables.
 *
 * "Digitized" refers to quantities produced after detector response + electronics logic:
 * ADC/TDC-like values, integrated charge proxies, derived timing, and readout coordinates.
 *
 * Key responsibilities implemented here:
 * - Storage (overwrite semantics) via \ref GDigitizedData::includeVariable()
 * - Run integration (summation semantics) via \ref GDigitizedData::accumulateVariable()
 * - Filtering of "streaming readout" (SRO) keys via \ref GDigitizedData::getIntObservablesMap()
 *   and \ref GDigitizedData::getDblObservablesMap()
 * - Convenience accessors for commonly used keys, notably \ref TIMEATELECTRONICS
 *
 * \note Threading: a single atomic counter is used only by the example factory
 * \ref GDigitizedData::create(). Regular instances have no shared mutable state.
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
///
/// \details
/// This counter is not intended for physics production; it exists for deterministic example data.
std::atomic<int> GDigitizedData::globalDigitizedDataCounter{0};

GDigitizedData::GDigitizedData(const std::shared_ptr<GOptions>& gopts, const GHit* ghit)
	: GBase(gopts, GDIGITIZED_DATA_LOGGER) {
	// Copy hit identity (sector/layer/component...) from the originating hit.
	//
	// Ownership note:
	// - ghit is not owned.
	// - gidentity is copied so the digitized object is self-contained.
	gidentity = ghit->getGID();
}

std::map<std::string, int> GDigitizedData::getIntObservablesMap(int which) const {
	// Filtering is based on whether the key is a streaming readout key (SRO) or not.
	//
	// which=0 -> return non-SRO variables (physics content / derived digitization results)
	// which=1 -> return only SRO variables (crate/slot/channel/timeAtElectronics/chargeAtElectronics)
	//
	// Rationale:
	// Some backends want to handle readout coordinates separately from physics-like observables.
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
	//
	// A variable is considered SRO if it matches one of the conventional keys defined in
	// \ref gdataConventions.h.
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
	// Store/overwrite a per-hit integer observable.
	//
	// Typical uses:
	// - integerized electronics payloads (TDC counts, ADC integer values)
	// - discrete indices and tags
	// - (also used for SRO keys: crate/slot/channel/timeAtElectronics)
	//
	// Overwrite semantics:
	// - repeated calls with the same key replace the stored value.
	log->info(2, "Including int variable ", vname, " with value ", value);
	intObservablesMap[vname] = value;
}

void GDigitizedData::includeVariable(const std::string& vname, double value) {
	// Store/overwrite a per-hit floating observable.
	//
	// Typical uses:
	// - calibrated energies
	// - ADC-like floating values
	// - continuous timing quantities
	//
	// Overwrite semantics:
	// - repeated calls with the same key replace the stored value.
	log->info(2, "Including double variable ", vname, " with value ", value);
	doubleObservablesMap[vname] = value;
}

void GDigitizedData::accumulateVariable(const std::string& vname, int value) {
	// Accumulate is meant for run-level integration: sum contributions into the same key.
	//
	// Summation semantics:
	// - if the key does not exist, it is created with 'value'
	// - otherwise, 'value' is added to the existing entry
	//
	// IMPORTANT:
	// - No normalization is performed (consumer should normalize if needed).
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
	//
	// Summation semantics:
	// - if the key does not exist, it is created with 'value'
	// - otherwise, 'value' is added to the existing entry
	//
	// IMPORTANT:
	// - No normalization is performed (consumer should normalize if needed).
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
	//
	// Rationale:
	// - TIMEATELECTRONICS is frequently accessed by consumers/backends.
	// - Returning a sentinel when missing avoids silent default insertion.
	//
	// \return TIMEATELECTRONICSNOTDEFINED if the key is not present.
	if (intObservablesMap.find(TIMEATELECTRONICS) == intObservablesMap.end()) { return TIMEATELECTRONICSNOTDEFINED; }
	log->info(2, "Getting TIMEATELECTRONICS from intObservablesMap.");
	return intObservablesMap[TIMEATELECTRONICS];
}

int GDigitizedData::getIntObservable(const std::string& varName) {
	// Access one integer observable by name.
	//
	// Error behavior:
	// - If missing, emit an error via the logger (using ERR_VARIABLENOTFOUND).
	// - Then return intObservablesMap[varName] (operator[]), which (if missing) would insert
	//   a default value. The assumption is that the logger's configuration controls whether
	//   execution is allowed to continue.
	if (intObservablesMap.find(varName) == intObservablesMap.end()) {
		log->error(ERR_VARIABLENOTFOUND,
		           "variable name <" + varName + "> not found in GDigitizedData::intObservablesMap");
	}
	return intObservablesMap[varName];
}

double GDigitizedData::getDblObservable(const std::string& varName) {
	// Access one double observable by name.
	//
	// Error behavior mirrors getIntObservable().
	if (doubleObservablesMap.find(varName) == doubleObservablesMap.end()) {
		log->error(ERR_VARIABLENOTFOUND,
		           "variable name <" + varName + "> not found in GDigitizedData::doubleObservablesMap");
	}
	return doubleObservablesMap[varName];
}

std::string GDigitizedData::getIdentityString() const {
	// Produces a concise representation:
	//   name1->value1, name2->value2, ...
	//
	// Usage:
	// - debugging and log labeling
	// - stable identity string for output tagging
	//
	// Precondition:
	// - identity vector is expected to be non-empty for objects constructed from valid hits.
	std::string identifierString;
	for (size_t i = 0; i < gidentity.size() - 1; i++) {
		identifierString += gidentity[i].getName() + "->" + std::to_string(gidentity[i].getValue()) + ", ";
	}
	identifierString += gidentity.back().getName() + "->" + std::to_string(gidentity.back().getValue());
	return identifierString;
}
