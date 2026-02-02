/**
 * \file gTrueInfoData.cc
 * \brief Implementation of the \ref GTrueInfoData class.
 *
 * \details
 * This translation unit provides the implementation for storing and manipulating "true"
 * (simulation-level) observables associated with **one hit**.
 *
 * Conceptually, a \ref GTrueInfoData object is built during event processing and later consumed by:
 * - output backends (to serialize truth information)
 * - run-level integrators (to build sums across hits/events)
 * - debugging/QA tools (to inspect hit provenance and content)
 *
 * Responsibilities implemented here:
 * - \ref GTrueInfoData::includeVariable "includeVariable()"
 *   stores per-hit observables with **overwrite** semantics (event-level filling).
 * - \ref GTrueInfoData::accumulateVariable "accumulateVariable()"
 *   integrates numeric observables with **summation** semantics (run-level integration).
 * - \ref GTrueInfoData::getIdentityString "getIdentityString()"
 *   formats the hit identity vector into a compact human-readable label.
 *
 * Storage model:
 * - Observables are stored as string-keyed maps (name \f$\rightarrow\f$ value), which allows detectors and
 *   plugins to extend the schema without requiring a recompile of the core library.
 *
 * \note Threading
 * Regular instances have no shared mutable state. The only global state in this compilation unit is
 *  \c globalTrueInfoDataCounter used by the example factory
 * \ref GTrueInfoData::create "create()", and it is an \c std::atomic to remain thread-safe.
 */

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
	// Intended usage:
	// - Called while constructing the truth record for the current hit during event processing.
	// - Used for step-averaged or hit-aggregated values such as energy deposition, positions, time,
	//   or any detector/plugin-defined numeric scalar.
	//
	// Semantics:
	// - Overwrite: repeated calls with the same key replace the previous value.
	//
	// Notes:
	// - The map key is an arbitrary string chosen by the producer. Consumers should treat keys as
	//   part of the detector/plugin schema and avoid hard-coded assumptions when possible.
	doubleObservablesMap[varName] = value;
	log->info(2, FUNCTION_NAME, " including ", varName, " in trueInfoDoublesVariablesMap with value: ", value);
}

void GTrueInfoData::includeVariable(const std::string& varName, std::string value) {
	// Store/overwrite a per-hit "true" string observable.
	//
	// Intended usage:
	// - Categorical/provenance labels: process name, volume name, particle name, generator tag, etc.
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
	// Typical usage:
	// - A run-level collector keeps a single \ref GTrueInfoData as an accumulator.
	// - Each event/hit contributes numeric values to that accumulator.
	//
	// Semantics (summation):
	// - First contribution creates the key with the provided value.
	// - Subsequent contributions add to the existing value (running sum).
	//
	// IMPORTANT:
	// - This method performs **summation only**. It does not compute means, rates, RMS, etc.
	// - If you need derived quantities (e.g. average energy per event), compute them in the consumer
	//   using the correct denominator (number of hits/events, time window, etc.).
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
	//
	// Intended usage:
	// - Debugging: quick "where did this hit occur?" label.
	// - Output labeling: stable per-hit identity tag for logs/records.
	//
	// Precondition:
	// - The identity vector is expected to be non-empty when the object is constructed from a valid GHit.
	std::string identifierString;
	for (size_t i = 0; i < gidentity.size() - 1; i++) {
		identifierString += gidentity[i].getName() + "->" + std::to_string(gidentity[i].getValue()) + ", ";
	}
	identifierString += gidentity.back().getName() + "->" + std::to_string(gidentity.back().getValue());
	return identifierString;
}
