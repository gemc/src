#pragma once

/**
 * \file gTrueInfoData.h
 * \brief Container for simulation-level observables associated with a single hit.
 *
 * \details
 * GTrueInfoData stores the "true" quantities produced at the simulation stage, before any
 * detector electronics or digitization transforms them into readout-like values.
 *
 * Typical examples include:
 * - deposited energy
 * - average position and time over the contributing steps
 * - categorical metadata such as process or volume names
 *
 * The container is intentionally schema-flexible. Instead of hard-coding a fixed bank layout,
 * observables are stored in keyed maps so that detectors and plugins can evolve their content
 * without requiring ABI changes in this module.
 *
 * Stored categories:
 * - \c doubleObservablesMap : numeric truth quantities
 * - \c stringVariablesMap   : string-based categorical or provenance metadata
 *
 * Usage modes:
 * - Event mode: create one object per hit and fill with
 *   \ref GTrueInfoData::includeVariable "includeVariable()".
 * - Run/integrated mode: use one object as an accumulator and update it with
 *   \ref GTrueInfoData::accumulateVariable "accumulateVariable()".
 *
 * Identity model:
 * - each object stores a copy of the hit identity vector extracted from GHit
 * - the identity is preserved independently of the originating hit lifetime
 * - a readable representation is available through
 *   \ref GTrueInfoData::getIdentityString "getIdentityString()"
 *
 * Threading:
 * - regular instances do not share mutable state
 * - the example/test factory \ref GTrueInfoData::create "create()" uses a static atomic counter
 */

#include <atomic>
#include <map>
#include <ostream>
#include <string>
#include <vector>

// gemc
#include "gbase.h"
#include "ghit.h"

/// Logger domain name used by GTrueInfoData.
constexpr const char* GTRUEDATA_LOGGER = "true_data";

namespace gtrue_data {

/**
 * \brief Defines the options subtree used by the true-data logger domain.
 *
 * \details
 * Higher-level modules can aggregate this option group into their own configuration bundles
 * so that verbosity and related behavior for GTrueInfoData can be controlled centrally.
 *
 * \return Options group rooted at \c GTRUEDATA_LOGGER.
 */
inline GOptions defineOptions() {
	auto goptions = GOptions(GTRUEDATA_LOGGER);
	return goptions;
}

} // namespace gtrue_data

/**
 * \brief Stores simulation-level observables for one hit.
 *
 * \details
 * A GTrueInfoData object represents the truth-side data model for one hit.
 * It is typically produced from tracking or stepping information and later attached to:
 * - a detector-level collection via GDataCollection
 * - an event-level collection via GEventDataCollection
 * - an integrated summary when used as an accumulator
 *
 * Main responsibilities:
 * - hold numeric truth observables
 * - hold string metadata associated with the hit
 * - preserve a copy of the detector identity for later labeling, debugging, and export
 *
 * Accumulation model:
 * - \ref GTrueInfoData::includeVariable "includeVariable()" overwrites a stored value for event filling
 * - \ref GTrueInfoData::accumulateVariable "accumulateVariable()" sums contributions for integration
 *
 * \note
 * Accumulation is purely additive. Any normalization, averaging, or rate computation belongs
 * in higher-level consumer code.
 */
class GTrueInfoData : public GBase<GTrueInfoData>
{
public:
	/**
	 * \brief Constructs the object and copies the hit identity from the source hit.
	 *
	 * \details
	 * The constructor initializes the base logging domain and copies the vector of GIdentifier
	 * entries from the provided hit so that this object becomes self-contained.
	 *
	 * Ownership and lifetime:
	 * - \p ghit is not owned
	 * - \p ghit only needs to remain valid during construction
	 * - after construction, this object no longer depends on the source hit
	 *
	 * \param gopts Shared options used to configure logging and related behavior.
	 * \param ghit  Source hit providing the identity vector.
	 */
	GTrueInfoData(const std::shared_ptr<GOptions>& gopts, const GHit* ghit);

	/**
	 * \brief Builds a readable identity string from the stored hit identifiers.
	 *
	 * \details
	 * The generated format is:
	 * \code
	 * name1->value1, name2->value2, ...
	 * \endcode
	 *
	 * This string is intended for:
	 * - logs
	 * - debugging output
	 * - human-readable summaries
	 *
	 * \return Identity string assembled from the stored \c gidentity vector.
	 */
	[[nodiscard]] std::string getIdentityString() const;

	/**
	 * \brief Stores or overwrites one numeric truth observable.
	 *
	 * \details
	 * This method is typically used during event-level filling.
	 * Repeated insertion with the same key replaces the previous value.
	 *
	 * Common examples include:
	 * - total deposited energy
	 * - average time
	 * - average coordinates
	 *
	 * \param varName Observable key.
	 * \param var     Numeric value to store.
	 */
	void includeVariable(const std::string& varName, double var);

	/**
	 * \brief Stores or overwrites one string truth observable.
	 *
	 * \details
	 * This method is intended for per-hit metadata that is better represented as text, such as:
	 * - process names
	 * - particle names
	 * - volume labels
	 * - provenance tags
	 *
	 * Repeated insertion with the same key replaces the previous value.
	 *
	 * \param varName Observable key.
	 * \param var     String value to store.
	 */
	void includeVariable(const std::string& varName, std::string var);

	/**
	 * \brief Accumulates a numeric observable into the current object.
	 *
	 * \details
	 * This method supports run-level or integrated accumulation.
	 *
	 * Behavior:
	 * - if the key does not exist, it is created with \p value
	 * - if the key already exists, \p value is added to the stored entry
	 *
	 * \param vname Observable key.
	 * \param value Contribution to add to the running sum.
	 */
	void accumulateVariable(const std::string& vname, double value);

	/**
	 * \brief Returns a copy of the numeric truth observables.
	 *
	 * \details
	 * Returning by value preserves encapsulation and prevents external mutation of the internal map.
	 *
	 * \return Copy of the double-valued observables map.
	 */
	[[nodiscard]] inline std::map<std::string, double> getDoubleVariablesMap() const {
		return doubleObservablesMap;
	}

	/**
	 * \brief Returns a copy of the string truth observables.
	 *
	 * \details
	 * These values are usually categorical or provenance-oriented and are typically not merged
	 * during run-level integration in GDataCollection.
	 *
	 * \return Copy of the string-valued observables map.
	 */
	[[nodiscard]] inline std::map<std::string, std::string> getStringVariablesMap() const {
		return stringVariablesMap;
	}

	/**
	 * \brief Creates deterministic example data for tests and examples.
	 *
	 * \details
	 * This helper is intended only for demonstrations and tests.
	 * It does not model real detector truth generation.
	 *
	 * The returned object contains a small conventional set of numeric variables:
	 * - \c totalEDeposited
	 * - \c avgTime
	 * - \c avgx
	 * - \c avgy
	 * - \c avgz
	 * - \c hitn
	 *
	 * Uniqueness across calls is derived from a static atomic counter.
	 *
	 * \param gopts Shared options.
	 * \return Newly created example object.
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
		true_info_data->includeVariable("hitn", counter);

		return true_info_data;
	}

private:
	/**
	 * \brief Numeric truth observables.
	 *
	 * \details
	 * This map stores scalar numeric values for either:
	 * - one event-level hit, or
	 * - one integrated accumulator entry, depending on usage
	 */
	std::map<std::string, double> doubleObservablesMap;

	/**
	 * \brief String truth observables.
	 *
	 * \details
	 * This map stores textual metadata associated with a hit, such as provenance or labels.
	 */
	std::map<std::string, std::string> stringVariablesMap;

	/**
	 * \brief Identity copied from the originating hit.
	 *
	 * \details
	 * The vector contains the detector-identifying indices used to uniquely label where the hit
	 * occurred, for example sector, layer, or component.
	 */
	std::vector<GIdentifier> gidentity;

	/**
	 * \brief Global example/test counter used by \ref GTrueInfoData::create "create()".
	 *
	 * \details
	 * This counter is only used to generate deterministic dummy content in examples and tests.
	 */
	static std::atomic<int> globalTrueInfoDataCounter;

protected:
	friend std::ostream& operator<<(std::ostream& os, const GTrueInfoData& data);
};