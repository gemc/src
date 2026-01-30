#pragma once

/**
 * \file gTrueInfoData.h
 * \brief Container for "true" (simulation-level) observables associated with one hit.
 *
 * \details
 * \ref GTrueInfoData stores *truth* information typically derived from Geant4 tracking:
 * energy deposition, step-averaged positions, time, particle/process metadata, etc.
 *
 * Observables are stored as named maps:
 * - \c doubleObservablesMap : numeric truth quantities (edep, x/y/z, time, ...)
 * - \c stringVariablesMap   : categorical/provenance values (process name, volume name, ...)
 *
 * Per-event vs per-run semantics:
 * - includeVariable(): sets/overwrites a variable for a single hit (event-level).
 * - accumulateVariable(): adds into a running sum (run-level integration).
 *
 * Identity:
 * Each \ref GTrueInfoData also stores the hit identity (\c gidentity), copied from \c GHit.
 * This is typically a vector of named indices (e.g. sector/layer/component) that uniquely identify
 * where the hit occurred.
 */

// c++
#include <string>
#include <map>
#include <vector>
#include <atomic>

// gemc
#include "ghit.h"
#include "gbase.h"

constexpr const char* GTRUEDATA_LOGGER = "true_data";

namespace gtrue_data {
/**
 * \brief Defines GOptions for the true-data logger domain.
 *
 * This is typically aggregated by higher-level option groups (event/run collections).
 */
inline GOptions defineOptions() {
	auto goptions = GOptions(GTRUEDATA_LOGGER);
	return goptions;
}
} // namespace gtrue_data

class GTrueInfoData : public GBase<GTrueInfoData>
{
public:
	/**
	 * \brief Construct true-hit data from a hit identity.
	 *
	 * The constructor copies the hit identity (\c GIdentifier vector) from \p ghit.
	 *
	 * \param gopts Shared options object used to configure logging and behavior.
	 * \param ghit  Pointer to the hit providing identity information (not owned).
	 */
	GTrueInfoData(const std::shared_ptr<GOptions>& gopts, const GHit* ghit);

	/**
	 * \brief Return a human-readable identity string for debugging and labeling.
	 *
	 * Format: \c name1->value1, name2->value2, ...
	 *
	 * \return Identity string assembled from the hit identifiers.
	 */
	[[nodiscard]] std::string getIdentityString() const;

	/**
	 * \brief Store/overwrite a numeric "true" observable for this hit.
	 *
	 * Typical variables include:
	 * - total energy deposition ("totalEDeposited")
	 * - average time ("avgTime")
	 * - average position ("avgx", "avgy", "avgz")
	 *
	 * \note If you also want integer truth observables, store them as doubles
	 * (implicit conversion) or add a dedicated integer map.
	 *
	 * \param varName Observable name/key.
	 * \param var     Value to store.
	 */
	void includeVariable(const std::string& varName, double var);

	/**
	 * \brief Store/overwrite a string "true" observable for this hit.
	 *
	 * Typical variables include process/volume/particle names or other provenance.
	 *
	 * \param varName Observable name/key.
	 * \param var     String value to store (moved into the map).
	 */
	void includeVariable(const std::string& varName, std::string var);

	/**
	 * \brief Accumulate a numeric observable into this object (run-level integration).
	 *
	 * If \p vname is not present, it is created with \p value.
	 * If present, \p value is added to the existing entry.
	 *
	 * This is typically used by run-level collectors that integrate hit contributions
	 * over many events.
	 *
	 * \param vname Observable name/key.
	 * \param value Contribution to add.
	 */
	void accumulateVariable(const std::string& vname, double value);

	/**
	 * \brief Get a copy of all numeric truth observables.
	 *
	 * Returning by value keeps the internal map encapsulated and avoids exposing mutable references.
	 *
	 * \return Copy of the double observables map.
	 */
	[[nodiscard]] inline std::map<std::string, double> getDoubleVariablesMap() const {
		return doubleObservablesMap;
	}

	/**
	 * \brief Get a copy of all string truth observables.
	 *
	 * \return Copy of the string observables map.
	 */
	[[nodiscard]] inline std::map<std::string, std::string> getStringVariablesMap() const {
		return stringVariablesMap;
	}

	/**
	 * \brief Test/example factory: create a true-hit object with deterministic dummy data.
	 *
	 * This method exists to support examples and unit tests. It generates a predictable
	 * set of variables using a thread-safe counter.
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

	/// Static thread-safe counter used only by \ref GTrueInfoData::create() (examples/tests).
	static std::atomic<int> globalTrueInfoDataCounter;
};
