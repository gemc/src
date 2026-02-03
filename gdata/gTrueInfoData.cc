/**
 * \file gTrueInfoData.cc
 * \brief Implementation of \ref GTrueInfoData.
 *
 * \details
 * See \ref gTrueInfoData.h for the authoritative API documentation, data-model notes,
 * and event-vs-run usage semantics.
 */

// See header for API docs.

#include "gTrueInfoData.h"
#include <string>
#include <utility>

/// Global counter used only by the static factory \ref GTrueInfoData::create "create()".
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
	// Ownership and lifetime:
	// - This constructor does not own the GHit pointer.
	// - We copy the identifier vector out of the hit so the GTrueInfoData is self-contained
	//   and safe to use after the originating hit object has gone out of scope.
	gidentity = ghit->getGID();
}

void GTrueInfoData::includeVariable(const std::string& varName, double value) {
	// Store/overwrite a per-hit "true" numeric observable.
	//
	// Semantics:
	// - Overwrite: repeated calls with the same key replace the previous value.
	doubleObservablesMap[varName] = value;
	log->info(2, FUNCTION_NAME, " including ", varName, " in trueInfoDoublesVariablesMap with value: ", value);
}

void GTrueInfoData::includeVariable(const std::string& varName, std::string value) {
	// Store/overwrite a per-hit "true" string observable.
	//
	// Semantics:
	// - Overwrite: repeated calls with the same key replace the previous value.
	//
	// Performance note:
	// - \p value is moved into the internal map to avoid an extra copy when callers pass temporaries.
	log->info(2, FUNCTION_NAME, " including ", varName, " in trueInfoStringVariablesMap  with value:", value);
	stringVariablesMap[varName] = std::move(value);
}

void GTrueInfoData::accumulateVariable(const std::string& vname, double value) {
	// Accumulate is meant for run-level (integrated) quantities.
	//
	// Semantics (summation):
	// - First contribution creates the key with the provided value.
	// - Subsequent contributions add to the existing value (running sum).
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
	// Build a concise identity label from the \c gidentity vector.
	//
	// Output format:
	//   name1->value1, name2->value2, ...
	std::string identifierString;
	for (size_t i = 0; i < gidentity.size() - 1; i++) {
		identifierString += gidentity[i].getName() + "->" + std::to_string(gidentity[i].getValue()) + ", ";
	}
	identifierString += gidentity.back().getName() + "->" + std::to_string(gidentity.back().getValue());
	return identifierString;
}
