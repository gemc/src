/**
 * \file gTrueInfoData.cc
 * \brief Implementation of the \ref GTrueInfoData class.
 *
 * \details
 * This translation unit implements the core mechanics for "truth" (simulation-level)
 * observables associated with a single hit:
 *
 * - \ref GTrueInfoData::includeVariable() stores per-hit variables (overwrite semantics).
 * - \ref GTrueInfoData::accumulateVariable() integrates variables (summation semantics),
 *   typically used by run-level collectors.
 * - \ref GTrueInfoData::getIdentityString() provides a compact identity label derived
 *   from the hit identifier vector (\c gidentity).
 *
 * The class stores observables using name->value maps to support detector- and plugin-defined
 * schemas without hard-coding variable layouts.
 *
 * \note Threading: the only global state in this compilation unit is the example/test counter
 * \ref GTrueInfoData::globalTrueInfoDataCounter, which is \c std::atomic for thread-safety.
 */

#include "gTrueInfoData.h"
#include <string>
#include <utility>

/// Global counter used only by the static factory \ref GTrueInfoData::create().
/// It is intentionally thread-safe to support examples and tests running in parallel.
///
/// \details
/// This counter is **not** used in physics production. It exists so that the toy factory can
/// generate deterministic, unique-ish values across multiple calls and threads.
std::atomic<int> GTrueInfoData::globalTrueInfoDataCounter{0};

GTrueInfoData::GTrueInfoData(const std::shared_ptr<GOptions>& gopts, const GHit* ghit)
	: GBase(gopts, GTRUEDATA_LOGGER) {
	// The "identity" is a vector of (name,value) pairs that uniquely identifies
	// the hit within a detector (e.g. sector/layer/component).
	//
	// Ownership note:
	// - This constructor does not own the GHit.
	// - We copy the identifier vector out of the hit so the GTrueInfoData is self-contained.
	gidentity = ghit->getGID();
}

void GTrueInfoData::includeVariable(const std::string& varName, double value) {
	// Store/overwrite a per-hit "true" numeric observable.
	//
	// Semantics:
	// - Event-level: called while constructing the hit record for the current event.
	// - The latest value wins: repeated calls with the same key overwrite.
	//
	// Typical variables:
	// - energy deposition, step-averaged kinematics, positions, time, etc.
	doubleObservablesMap[varName] = value;
	log->info(2, FUNCTION_NAME, " including ", varName, " in trueInfoDoublesVariablesMap with value: ", value);
}

void GTrueInfoData::includeVariable(const std::string& varName, std::string value) {
	// Store/overwrite a per-hit "true" string observable.
	//
	// Typical variables:
	// - process name, volume name, particle name, or other provenance labels.
	//
	// Performance:
	// - value is moved into the map to avoid an extra copy when callers pass temporaries.
	log->info(2, FUNCTION_NAME, " including ", varName, " in trueInfoStringVariablesMap  with value:", value);
	stringVariablesMap[varName] = std::move(value);
}

void GTrueInfoData::accumulateVariable(const std::string& vname, double value) {
	// Accumulate is meant for run-level (integrated) quantities.
	//
	// Semantics:
	// - First contribution creates the key with the provided value.
	// - Subsequent contributions add to the existing value (running sum).
	//
	// IMPORTANT:
	// - This method performs **summation** only. It does not compute means, rates, or RMS.
	// - If you need derived quantities (e.g. average energy deposition per event),
	//   normalize in the consumer using the appropriate denominator.
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
	//
	// Usage:
	// - debugging: quick "where did this hit happen?" label
	// - output: stable identity label for logs and record tags
	//
	// Precondition:
	// - The identity vector is expected to be non-empty when the object is constructed
	//   from a valid GHit.
	std::string identifierString;
	for (size_t i = 0; i < gidentity.size() - 1; i++) {
		identifierString += gidentity[i].getName() + "->" + std::to_string(gidentity[i].getValue()) + ", ";
	}
	identifierString += gidentity.back().getName() + "->" + std::to_string(gidentity.back().getValue());
	return identifierString;
}
