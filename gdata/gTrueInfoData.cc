/**
 * \file gTrueInfoData.cc
 * \brief Implementation of the GTrueInfoData class.
 *
 * This translation unit implements the methods that populate and integrate
 * ("accumulate") true-hit observables, and it provides a human-readable
 * identity string derived from the hit identifiers.
 */

#include "gTrueInfoData.h"
#include <string>
#include <utility>

/// Global counter used only by the static factory \ref GTrueInfoData::create().
/// It is intentionally thread-safe to support examples and tests running in parallel.
std::atomic<int> GTrueInfoData::globalTrueInfoDataCounter{0};

GTrueInfoData::GTrueInfoData(const std::shared_ptr<GOptions>& gopts, const GHit* ghit)
	: GBase(gopts, GTRUEDATA_LOGGER) {
	// The "identity" is a vector of (name,value) pairs that uniquely identifies
	// the hit within a detector (e.g. sector/layer/component).
	// This is copied from the underlying GHit.
	gidentity = ghit->getGID();
}

void GTrueInfoData::includeVariable(const std::string& varName, double value) {
	// Store/overwrite a per-hit "true" observable.
	// Typical use: quantities that come from Geant4 tracking (edep, position, time, etc.)
	doubleObservablesMap[varName] = value;
	log->info(2, FUNCTION_NAME, " including ", varName, " in trueInfoDoublesVariablesMap with value: ", value);
}

void GTrueInfoData::includeVariable(const std::string& varName, std::string value) {
	// Store/overwrite a per-hit "true" string observable.
	// Typical use: categorical or provenance information (process name, volume name, particle name, etc.)
	log->info(2, FUNCTION_NAME, " including ", varName, " in trueInfoStringVariablesMap  with value:", value);
	stringVariablesMap[varName] = std::move(value);
}

void GTrueInfoData::accumulateVariable(const std::string& vname, double value) {
	// Accumulate is meant for run-level (integrated) quantities: the same variable is
	// repeatedly contributed to across events/hits, and the map entry becomes a running sum.
	//
	// Important: this method does not attempt to normalize (e.g. compute averages).
	// Normalization (divide by number of events/hits) should be done by the consumer.
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
