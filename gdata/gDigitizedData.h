#pragma once

/**
 * \file GDigitizedData.h
 * \brief Container for digitized observables associated with one simulated hit.
 *
 * \details
 * \ref GDigitizedData represents the *post-digitization* view of a hit:
 * quantities produced by detector response + electronics logic. It stores
 * named observables in maps so that digitization plugins can define custom
 * variables without hard-coding a schema.
 *
 * Data organization:
 * - \c intObservablesMap    : scalar integer observables (indices, integerized electronics, etc.)
 * - \c doubleObservablesMap : scalar floating observables (ADC-like values, energies, calibrated times)
 * - \c arrayIntObservablesMap / \c arrayDoubleObservablesMap : optional vector-valued observables
 *
 * Per-event vs per-run semantics:
 * - includeVariable(): sets/overwrites the observable for a single hit/event.
 * - accumulateVariable(): adds the value into the stored observable (run-level integration).
 *
 * Streaming-readout (SRO) keys:
 * The conventional electronics keys defined in \ref gdataConventions.h
 * (crate/slot/channel/timeAtElectronics/chargeAtElectronics) are treated specially
 * by the filtering accessors \ref GDigitizedData::getIntObservablesMap() and \ref GDigitizedData::getDblObservablesMap().
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

/// Logger domain name used by \ref GDigitizedData (controls verbosity/category in \ref GLogger).
constexpr const char* GDIGITIZED_DATA_LOGGER = "digitized_data";

namespace gdigi_data {
/**
 * \brief Defines GOptions for the digitized-data logger domain.
 *
 * This is typically aggregated by higher-level option groups (event/run collections).
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
 * Stores named integer/double observables in maps, plus optional array-valued observables.
 * Supports run-level accumulation via accumulateVariable().
 */
class GDigitizedData : public GBase<GDigitizedData>
{
public:
	/**
	 * \brief Construct digitized data from a hit identity.
	 *
	 * The constructor copies the hit identity (\c GIdentifier vector) from \p ghit.
	 * The identity is later available in textual form via \ref getIdentityString().
	 *
	 * \param gopts Shared options object used to configure logging and behavior.
	 * \param ghit  Pointer to the hit providing identity information (not owned).
	 */
	GDigitizedData(const std::shared_ptr<GOptions>& gopts, const GHit* ghit);

	/**
	 * \brief Return a human-readable identity string for debugging and labeling.
	 *
	 * Format: \c name1->value1, name2->value2, ...
	 *
	 * \return Identity string assembled from the hit identifiers.
	 */
	[[nodiscard]] std::string getIdentityString() const;

	/**
	 * \name Per-hit insertion API
	 * \brief Store/overwrite observables for a single hit.
	 *
	 * These are typically called by digitization code that computes observables
	 * for the current hit/event.
	 * @{
	 */

	/// Store/overwrite an integer observable for this hit (event-level).
	void includeVariable(const std::string& vname, int value);

	/// Store/overwrite a double observable for this hit (event-level).
	void includeVariable(const std::string& vname, double value);
	/** @} */

	/**
	 * \name Run-level integration API
	 * \brief Accumulate observables across hits/events.
	 *
	 * These are typically called when building run-integrated summaries, i.e.
	 * a single \ref GDigitizedData becomes the running accumulator.
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
	 * Filtering is based on whether a key is considered "streaming readout" (SRO):
	 * - \p which = 0: returns non-SRO variables (physics content)
	 * - \p which = 1: returns only SRO variables (crate/slot/channel/time/charge)
	 *
	 * \param which Filter mode (0 = non-SRO, 1 = SRO only).
	 * \return A filtered copy of the integer observables.
	 */
	[[nodiscard]] std::map<std::string, int> getIntObservablesMap(int which) const;

	/**
	 * \brief Return a filtered copy of the double observables map.
	 *
	 * Filtering uses the same semantics as \ref getIntObservablesMap().
	 *
	 * \param which Filter mode (0 = non-SRO, 1 = SRO only).
	 * \return A filtered copy of the double observables.
	 */
	[[nodiscard]] std::map<std::string, double> getDblObservablesMap(int which) const;

	/**
	 * \brief Convenience accessor for \c TIMEATELECTRONICS.
	 *
	 * \return The time at electronics if present, otherwise \c TIMEATELECTRONICSNOTDEFINED.
	 */
	int getTimeAtElectronics();

	/**
	 * \brief Retrieve one integer observable by name.
	 *
	 * If the key is missing, an error is emitted via \c GLogger and then the current
	 * map value is returned (behavior depends on logger configuration).
	 *
	 * \param varName Observable name/key.
	 * \return The stored integer value.
	 */
	int getIntObservable(const std::string& varName);

	/**
	 * \brief Retrieve one double observable by name.
	 *
	 * If the key is missing, an error is emitted via \c GLogger and then the current
	 * map value is returned (behavior depends on logger configuration).
	 *
	 * \param varName Observable name/key.
	 * \return The stored double value.
	 */
	double getDblObservable(const std::string& varName);

	/**
	 * \brief Get the array-valued integer observables map.
	 *
	 * Array observables are optional and may be used for waveform-like quantities,
	 * time-slices, multi-sample payloads, etc.
	 *
	 * \return A copy of the array integer observables map.
	 */
	[[nodiscard]] inline std::map<std::string, std::vector<int>> getArrayIntObservablesMap() const {
		return arrayIntObservablesMap;
	}

	/**
	 * \brief Get the array-valued double observables map.
	 *
	 * \return A copy of the array double observables map.
	 */
	[[nodiscard]] inline std::map<std::string, std::vector<double>> getArrayDblObservablesMap() const {
		return arrayDoubleObservablesMap;
	}

	/**
	 * \brief Test/example factory: create a digitized hit with deterministic dummy data.
	 *
	 * This method exists to support examples and unit tests. It does not represent
	 * real detector digitization; instead it generates predictable values using a
	 * thread-safe counter.
	 *
	 * The returned object includes both SRO keys (crate/slot/channel/time) and one
	 * non-SRO observable ("adc").
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
	 * \param varName Observable name/key.
	 * \param which   0 = non-SRO, 1 = SRO only.
	 * \return True if the variable should be included under the requested filter.
	 */
	[[nodiscard]] static bool validVarName(const std::string& varName, int which);

	/// Static thread-safe counter used only by \ref GDigitizedData::create() (examples/tests).
	static std::atomic<int> globalDigitizedDataCounter;
};
