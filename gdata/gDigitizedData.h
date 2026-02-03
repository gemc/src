#pragma once

/**
 * \file GDigitizedData.h
 * \brief Container for digitized observables associated with one simulated hit.
 *
 * \details
 * \ref GDigitizedData represents the *post-digitization* view of a hit: quantities produced
 * after detector response and electronics logic.
 *
 * The design goal is schema flexibility: digitization plugins can define custom observables
 * using string keys without requiring the core library to hard-code bank layouts.
 *
 * ## Stored observable categories
 * - \c intObservablesMap    : scalar integer observables (indices, integerized electronics, etc.)
 * - \c doubleObservablesMap : scalar floating observables (ADC-like values, energies, calibrated times)
 * - \c arrayIntObservablesMap / \c arrayDoubleObservablesMap : optional vector-valued observables
 *
 * ## Per-event vs per-run semantics
 * - \ref GDigitizedData::includeVariable "includeVariable()"
 *   sets/overwrites the observable for a single hit (event-level filling).
 * - \ref GDigitizedData::accumulateVariable "accumulateVariable()"
 *   adds the value into the stored observable (run-level integration).
 *
 * ## Streaming-readout (SRO) keys
 * The conventional readout keys defined in \ref gdataConventions.h :
 * - crate, slot, channel, timeAtElectronics, chargeAtElectronics
 *
 * are treated specially by the filtering accessors:
 * - \ref GDigitizedData::getIntObservablesMap "getIntObservablesMap()"
 * - \ref GDigitizedData::getDblObservablesMap "getDblObservablesMap()"
 *
 * This supports backends that want to separate "readout addressing" from "physics-like" observables.
 */

// c++
#include <string>
#include <map>
#include <vector>
#include <atomic>

// gemc
#include "ghit.h"
#include "gbase.h"

// gdata
#include "gdataConventions.h" // CRATESTRINGID, SLOTSTRINGID, CHANNELSTRINGID, TIMEATELECTRONICS, ...

/// Logger domain name used by \ref GDigitizedData (controls verbosity/category in GLogger).
constexpr const char* GDIGITIZED_DATA_LOGGER = "digitized_data";

namespace gdigi_data {
/**
 * \brief Defines GOptions for the digitized-data logger domain.
 *
 * \details
 * Higher-level aggregators (event/run collections) typically include this in their
 * composite option groups so that digitized-data logging can be enabled/controlled consistently.
 *
 * \return An options group rooted at the \ref GDIGITIZED_DATA_LOGGER domain.
 */
inline GOptions defineOptions() {
	auto goptions = GOptions(GDIGITIZED_DATA_LOGGER);
	return goptions;
}
} // namespace gdigi_data

/**
 * \brief Container for digitized (electronics-level) observables for one hit.
 *
 * \details
 * A \ref GDigitizedData instance corresponds to *one hit* after digitization.
 * It is designed to be schema-flexible (map-based storage) while still allowing:
 * - deterministic inspection via map iteration
 * - selective export of readout keys (SRO filtering)
 * - run-level integration via summation of scalars
 *
 * Common usage patterns:
 * 1) Event-level: create a new instance per hit; fill using \ref GDigitizedData::includeVariable "includeVariable()".
 * 2) Run-level: keep a single instance as an accumulator; integrate contributions with
 *    \ref GDigitizedData::accumulateVariable "accumulateVariable()".
 *
 * \note Accumulation is summation only; compute averages/rates in the consumer if needed.
 */
class GDigitizedData : public GBase<GDigitizedData>
{
public:
	/**
	 * \brief Construct digitized data by copying identity from a hit.
	 *
	 * \details
	 * The constructor copies the hit identity (\c GIdentifier vector) from \p ghit.
	 * The identity can be rendered as a human-readable string via
	 * \ref GDigitizedData::getIdentityString "getIdentityString()".
	 *
	 * Ownership:
	 * - \p ghit is not owned and only needs to be valid during construction.
	 *
	 * \param gopts Shared options object used to configure logging and behavior.
	 * \param ghit  Pointer to the hit providing identity information (not owned).
	 */
	GDigitizedData(const std::shared_ptr<GOptions>& gopts, const GHit* ghit);

	/**
	 * \brief Return a human-readable identity string for debugging and labeling.
	 *
	 * \details
	 * Format:
	 * \code
	 *   name1->value1, name2->value2, ...
	 * \endcode
	 *
	 * \return Identity string assembled from the hit identifiers.
	 */
	[[nodiscard]] std::string getIdentityString() const;

	/**
	 * \name Per-hit insertion API
	 * \brief Store/overwrite observables for a single hit.
	 *
	 * \details
	 * These methods are typically called by digitization logic that computes observables
	 * for the current hit and wants to attach them to the output record.
	 *
	 * Overwrite semantics:
	 * - Calling includeVariable() with an existing key replaces the stored value.
	 * @{
	 */

	/// Store/overwrite an integer observable for this hit (event-level).
	void includeVariable(const std::string& vname, int value);

	/// Store/overwrite a double observable for this hit (event-level).
	void includeVariable(const std::string& vname, double value);
	/** @} */

	/**
	 * \name Run-level integration API
	 * \brief Accumulate observables across hits/events (summation).
	 *
	 * \details
	 * These are used when a single \ref GDigitizedData is serving as an accumulator.
	 * - If the key does not exist, it is created.
	 * - If it exists, the new contribution is added to the stored value.
	 *
	 * \note No normalization is performed.
	 * @{
	 */

	/// Accumulate an integer observable (run-level integration).
	void accumulateVariable(const std::string& vname, int value);

	/// Accumulate a double observable (run-level integration).
	void accumulateVariable(const std::string& vname, double value);
	/** @} */

	/**
	 * \brief Return a filtered copy of the integer observables map.
	 *
	 * \details
	 * Filtering is based on whether a key is considered "streaming readout" (SRO):
	 * - \p which = 0: returns non-SRO variables (digitization outputs / physics-like quantities)
	 * - \p which = 1: returns only SRO variables (crate/slot/channel/timeAtElectronics/chargeAtElectronics)
	 *
	 * \param which Filter mode (0 = non-SRO, 1 = SRO only).
	 * \return A filtered copy of the integer observables.
	 */
	[[nodiscard]] std::map<std::string, int> getIntObservablesMap(int which) const;

	/**
	 * \brief Return a filtered copy of the double observables map.
	 *
	 * \details
	 * Uses the same filtering semantics as
	 * \ref GDigitizedData::getIntObservablesMap "getIntObservablesMap()".
	 *
	 * \param which Filter mode (0 = non-SRO, 1 = SRO only).
	 * \return A filtered copy of the double observables.
	 */
	[[nodiscard]] std::map<std::string, double> getDblObservablesMap(int which) const;

	/**
	 * \brief Convenience accessor for \c TIMEATELECTRONICS.
	 *
	 * \details
	 * If the key is not present, returns \ref TIMEATELECTRONICSNOTDEFINED (a sentinel).
	 * This avoids quietly inserting defaults and makes missing-data bugs easier to detect.
	 *
	 * \return The time at electronics if present, otherwise \c TIMEATELECTRONICSNOTDEFINED.
	 */
	int getTimeAtElectronics();

	/**
	 * \brief Retrieve one integer observable by name.
	 *
	 * \details
	 * If the key is missing, an error is emitted via GLogger using \ref ERR_VARIABLENOTFOUND.
	 * Return behavior after logging depends on the logger configuration (fatal vs non-fatal).
	 *
	 * \param varName Observable name/key.
	 * \return The stored integer value.
	 */
	int getIntObservable(const std::string& varName);

	/**
	 * \brief Retrieve one double observable by name.
	 *
	 * \details
	 * If the key is missing, an error is emitted via GLogger using \ref ERR_VARIABLENOTFOUND.
	 *
	 * \param varName Observable name/key.
	 * \return The stored double value.
	 */
	double getDblObservable(const std::string& varName);

	/**
	 * \brief Get the array-valued integer observables map.
	 *
	 * \details
	 * Array observables are optional and may represent:
	 * - waveforms
	 * - time slices / samples
	 * - multi-hit or multi-sample payloads
	 *
	 * \return A copy of the array integer observables map.
	 */
	[[nodiscard]] inline std::map<std::string, std::vector<int>> getArrayIntObservablesMap() const {
		return arrayIntObservablesMap;
	}

	/**
	 * \brief Get the array-valued double observables map.
	 *
	 * \details
	 * No filtering is applied to array-valued maps; their interpretation is producer-defined.
	 *
	 * \return A copy of the array double observables map.
	 */
	[[nodiscard]] inline std::map<std::string, std::vector<double>> getArrayDblObservablesMap() const {
		return arrayDoubleObservablesMap;
	}

	/**
	 * \brief Test/example factory: create a digitized hit with deterministic dummy data.
	 *
	 * \details
	 * This method exists to support examples and unit tests. It does not represent
	 * real detector digitization; instead it generates predictable values using a
	 * thread-safe counter.
	 *
	 * The returned object includes:
	 * - SRO keys: crate/slot/channel/timeAtElectronics
	 * - one non-SRO observable: "adc"
	 *
	 * \param gopts Shared options.
	 * \return A new digitized hit object.
	 */
	static std::unique_ptr<GDigitizedData> create(const std::shared_ptr<GOptions>& gopts) {
		auto hit       = GHit::create(gopts);
		auto digi_data = std::make_unique<GDigitizedData>(gopts, hit);
		auto counter   = globalDigitizedDataCounter.fetch_add(1, std::memory_order_relaxed);

		digi_data->includeVariable(CRATESTRINGID, counter % 10);
		digi_data->includeVariable(SLOTSTRINGID, counter % 20);
		digi_data->includeVariable(CHANNELSTRINGID, counter);
		digi_data->includeVariable(TIMEATELECTRONICS, counter * 5);
		digi_data->includeVariable("adc", counter * 0.1);
		return digi_data;
	}

private:
	/// Scalar integer observables for this digitized hit.
	std::map<std::string, int> intObservablesMap;

	/// Scalar double observables for this digitized hit.
	std::map<std::string, double> doubleObservablesMap;

	/// Optional array-valued integer observables.
	std::map<std::string, std::vector<int>> arrayIntObservablesMap;

	/// Optional array-valued double observables.
	std::map<std::string, std::vector<double>> arrayDoubleObservablesMap;

	/// Identity extracted from the originating hit.
	std::vector<GIdentifier> gidentity;

	/**
	 * \brief Helper used by filtering accessors to decide whether a key is returned.
	 *
	 * \details
	 * A key is considered SRO if it matches the conventional strings defined in
	 * \ref gdataConventions.h. The filter mode is:
	 * - which=0: include non-SRO keys only
	 * - which=1: include SRO keys only
	 *
	 * \param varName Observable name/key.
	 * \param which   0 = non-SRO, 1 = SRO only.
	 * \return True if the variable should be included under the requested filter.
	 */
	[[nodiscard]] static bool validVarName(const std::string& varName, int which);

	/// Static thread-safe counter used only by \ref GDigitizedData::create "create()" (examples/tests).
	static std::atomic<int> globalDigitizedDataCounter;
};
