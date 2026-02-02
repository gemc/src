#pragma once

/**
 * \file gTrueInfoData.h
 * \brief Container for "true" (simulation-level) observables associated with one hit.
 *
 * \details
 * \ref GTrueInfoData stores *truth* information typically derived from Geant4 tracking:
 * energy deposition, step-averaged positions, time, particle/process metadata, etc.
 *
 * ## Why maps?
 * Observables are stored as name->value maps to support detector/digitization plugins that:
 * - define custom variables without recompiling the core library
 * - evolve their schema over time without breaking binary interfaces
 *
 * ## Stored observable categories
 * - \c doubleObservablesMap : numeric truth quantities (edep, x/y/z, time, etc.)
 * - \c stringVariablesMap   : categorical/provenance values (process name, volume name, ...)
 *
 * ## Per-event vs per-run semantics
 * - \ref GTrueInfoData::includeVariable "includeVariable()"
 *   sets/overwrites a variable for a single hit (event-level).
 * - \ref GTrueInfoData::accumulateVariable "accumulateVariable()"
 *   adds into a running sum (run-level integration).
 *
 * ## Identity
 * Each \ref GTrueInfoData stores the hit identity (\c gidentity), copied from \ref GHit.
 * This is typically a vector of named indices (e.g. sector/layer/component) that uniquely identify
 * where the hit occurred. The identity is intended to be stable and human-readable via
 * \ref GTrueInfoData::getIdentityString "getIdentityString()".
 *
 * \note Threading
 * - Regular instances have no shared mutable state.
 * - The static factory \ref GTrueInfoData::create "create()" uses
 *   \c globalTrueInfoDataCounter which is atomic.
 */

// c++
#include <string>
#include <map>
#include <vector>
#include <atomic>

// gemc
#include "ghit.h"
#include "gbase.h"

/// Logger domain name used by \ref GTrueInfoData (controls verbosity/category in \ref GLogger).
constexpr const char* GTRUEDATA_LOGGER = "true_data";

namespace gtrue_data {
/**
 * \brief Defines \ref GOptions for the true-data logger domain.
 *
 * \details
 * This helper allows higher-level option aggregators (event/run collections) to pull in
 * configuration for this logger domain without knowing details about how \ref GLogger is set up.
 *
 * Typical usage:
 * \code
 *   GOptions opts("some_domain");
 *   opts += gtrue_data::defineOptions();
 * \endcode
 *
 * \return An options group rooted at the \ref GTRUEDATA_LOGGER domain.
 */
inline GOptions defineOptions() {
	auto goptions = GOptions(GTRUEDATA_LOGGER);
	return goptions;
}
} // namespace gtrue_data

/**
 * \brief Container for true (simulation-level) observables for one hit.
 *
 * \details
 * A \ref GTrueInfoData instance conceptually corresponds to *one simulated hit*.
 *
 * - It stores numeric and string observables keyed by name.
 * - It stores an identity vector derived from \ref GHit, typically encoding geometry indices.
 *
 * The container supports two usage patterns:
 * 1) **Event-level storage**: create a new instance per hit and populate it using
 *    \ref GTrueInfoData::includeVariable "includeVariable()".
 * 2) **Run-level integration**: keep a single instance as an accumulator and call
 *    \ref GTrueInfoData::accumulateVariable "accumulateVariable()"
 *    to sum contributions across hits/events.
 *
 * \note Accumulation is summation only; do not expect averages unless you compute them externally.
 */
class GTrueInfoData : public GBase<GTrueInfoData>
{
public:
	/**
	 * \brief Construct true-hit data by copying identity from a hit.
	 *
	 * \details
	 * - Copies the hit identity vector (\c GIdentifier list) from \p ghit.
	 * - Initializes the base logger domain to \ref GTRUEDATA_LOGGER.
	 *
	 * Ownership:
	 * - \p ghit is **not owned**; it must remain valid only for the duration of the constructor.
	 *
	 * \param gopts Shared options object used to configure logging and behavior.
	 * \param ghit  Pointer to the hit providing identity information (not owned).
	 */
	GTrueInfoData(const std::shared_ptr<GOptions>& gopts, const GHit* ghit);

	/**
	 * \brief Return a human-readable identity string for debugging and labeling.
	 *
	 * \details
	 * Format:
	 * \code
	 *   name1->value1, name2->value2, ...
	 * \endcode
	 *
	 * The underlying identity is the \c gidentity vector copied from the \ref GHit.
	 *
	 * \return Identity string assembled from the hit identifiers.
	 */
	[[nodiscard]] std::string getIdentityString() const;

	/**
	 * \brief Store/overwrite a numeric "true" observable for this hit.
	 *
	 * \details
	 * Overwrite semantics:
	 * - If \p varName already exists, the stored value is replaced.
	 * - If it does not exist, a new entry is created.
	 *
	 * Typical numeric truth keys (examples, not enforced):
	 * - "totalEDeposited"
	 * - "avgTime"
	 * - "avgx", "avgy", "avgz"
	 *
	 * \param varName Observable name/key.
	 * \param var     Value to store.
	 */
	void includeVariable(const std::string& varName, double var);

	/**
	 * \brief Store/overwrite a string "true" observable for this hit.
	 *
	 * \details
	 * String observables are typically used for categorical metadata such as:
	 * - physics process name
	 * - volume name
	 * - particle name
	 * - provenance tags
	 *
	 * Semantics:
	 * - Overwrite: repeated calls with the same key replace the stored value.
	 *
	 * \param varName Observable name/key.
	 * \param var     String value to store (moved into the map).
	 */
	void includeVariable(const std::string& varName, std::string var);

	/**
	 * \brief Accumulate a numeric observable into this object (run-level integration).
	 *
	 * \details
	 * Summation semantics:
	 * - If \p vname is absent, it is created with \p value.
	 * - If present, \p value is added to the existing entry.
	 *
	 * This method is typically used when integrating many hits/events into a run-level summary.
	 *
	 * \param vname Observable name/key.
	 * \param value Contribution to add.
	 */
	void accumulateVariable(const std::string& vname, double value);

	/**
	 * \brief Get a copy of all numeric truth observables.
	 *
	 * \details
	 * Returning by value keeps the internal storage encapsulated and avoids exposing a mutable reference.
	 * For very large maps, consider whether a const reference accessor is desirable at higher layers.
	 *
	 * \return Copy of the double observables map.
	 */
	[[nodiscard]] inline std::map<std::string, double> getDoubleVariablesMap() const {
		return doubleObservablesMap;
	}

	/**
	 * \brief Get a copy of all string truth observables.
	 *
	 * \details
	 * String observables are typically per-hit categorical/provenance values and are not accumulated
	 * by \ref GDataCollection in run mode.
	 *
	 * \return Copy of the string observables map.
	 */
	[[nodiscard]] inline std::map<std::string, std::string> getStringVariablesMap() const {
		return stringVariablesMap;
	}

	/**
	 * \brief Test/example factory: create a true-hit object with deterministic dummy data.
	 *
	 * \details
	 * This method exists to support examples and unit tests. It does **not** represent real
	 * physics truth generation. Values are created using a thread-safe counter so that:
	 * - each call produces different values
	 * - behavior is deterministic given call order
	 *
	 * The returned object includes a small set of conventional truth keys used in examples:
	 * - "totalEDeposited", "avgTime", "avgx", "avgy", "avgz", and "hitn".
	 *
	 * \param gopts Shared options.
	 * \return Newly created true-hit object.
	 */
	static std::unique_ptr<GTrueInfoData> create(const std::shared_ptr<GOptions>& gopts) {
		auto hit            = GHit::create(gopts);
		auto true_info_data = std::make_unique<GTrueInfoData>(gopts, hit);
		auto counter        = globalTrueInfoDataCounter.fetch_add(1, std::memory_order_relaxed);

		true_info_data->includeVariable("totalEDeposited", counter * 0.1);
		true_info_data->includeVariable("avgTime", counter * 1.0);
		true_info_data->includeVariable("avgx", counter * 0.01);
		true_info_data->includeVariable("avgy", counter * 0.02);
		true_info_data->includeVariable("avgz", counter * 0.03);

		// Stored as double by implicit conversion; useful as a simple monotonically increasing tag.
		true_info_data->includeVariable("hitn", counter);

		return true_info_data;
	}

private:
	/// Numeric truth observables (per-hit or run-integrated depending on usage).
	std::map<std::string, double> doubleObservablesMap;

	/// String truth observables (per-hit metadata/provenance).
	std::map<std::string, std::string> stringVariablesMap;

	/// Identity extracted from the originating hit (vector of named indices).
	std::vector<GIdentifier> gidentity;

	/// Static thread-safe counter used only by \ref GTrueInfoData::create "create()" (examples/tests).
	static std::atomic<int> globalTrueInfoDataCounter;
};
