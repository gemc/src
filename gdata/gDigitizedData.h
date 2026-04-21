#pragma once

/**
 * \file gDigitizedData.h
 * \brief Container for digitized observables associated with one simulated hit.
 *
 * \details
 * GDigitizedData represents the post-digitization view of a hit.
 * It stores the quantities produced after detector-response and electronics logic have transformed
 * simulation truth into readout-oriented observables.
 *
 * The class is intentionally schema-flexible:
 * - scalar values are stored in keyed maps
 * - optional array-valued observables can represent richer payloads such as samples or waveforms
 * - detector-specific content can be extended without changing the core library ABI
 *
 * Stored categories:
 * - \c intObservablesMap         : integer-valued scalar observables
 * - \c doubleObservablesMap      : floating-point scalar observables
 * - \c arrayIntObservablesMap    : integer arrays
 * - \c arrayDoubleObservablesMap : floating-point arrays
 *
 * Identity:
 * - the object copies the hit identity from the source GHit
 * - the identity is preserved independently of the originating hit lifetime
 *
 * Event and integration semantics:
 * - \ref GDigitizedData::includeVariable "includeVariable()" stores or overwrites per-hit values
 * - \ref GDigitizedData::accumulateVariable "accumulateVariable()" adds contributions for integrated use
 *
 * Streaming-readout filtering:
 * The conventional keys defined in gdataConventions.h :
 * - crate
 * - slot
 * - channel
 * - timeAtElectronics
 * - chargeAtElectronics
 *
 * are treated as streaming-readout identifiers and can be separated from non-SRO observables with:
 * - \ref GDigitizedData::getIntObservablesMap "getIntObservablesMap()"
 * - \ref GDigitizedData::getDblObservablesMap "getDblObservablesMap()"
 */

#include <atomic>
#include <map>
#include <ostream>
#include <string>
#include <vector>

// gemc
#include "gbase.h"
#include "ghit.h"

// gdata
#include "gdataConventions.h"

/// Logger domain name used by GDigitizedData.
constexpr const char* GDIGITIZED_DATA_LOGGER = "digitized_data";

namespace gdigi_data {

/**
 * \brief Defines the options subtree used by the digitized-data logger domain.
 *
 * \details
 * Higher-level modules can aggregate this option group into their own configuration bundles
 * so that verbosity and related behavior for GDigitizedData can be controlled centrally.
 *
 * \return Options group rooted at \c GDIGITIZED_DATA_LOGGER.
 */
inline GOptions defineOptions() {
	auto goptions = GOptions(GDIGITIZED_DATA_LOGGER);
	return goptions;
}

} // namespace gdigi_data

/**
 * \defgroup gdata_digitized_data GData digitized hit data
 * \brief Electronics-level observables stored for a single hit.
 *
 * \details
 * This topic documents the hit-local digitized data model used after detector response and
 * electronics processing. It covers scalar and array observables, filtering of SRO keys,
 * and accumulation semantics used for integrated containers.
 */

/**
 * \brief Stores digitized, electronics-level observables for one hit.
 * \ingroup gdata_digitized_data
 *
 * \details
 * A GDigitizedData instance models one hit after detector-response and digitization processing.
 * It is typically attached to:
 * - a detector-level container through GDataCollection
 * - an event-level container through GEventDataCollection
 * - a run/integrated container when used as an accumulator
 *
 * Main responsibilities:
 * - hold integer and floating scalar observables
 * - optionally hold array-valued observables
 * - provide filtered access that separates streaming-readout keys from physics-like content
 *
 * Typical usage:
 * - Event mode : fill one object per hit with
 *   \ref GDigitizedData::includeVariable "includeVariable()"
 * - Integrated mode : keep one object as an accumulator and update it with
 *   \ref GDigitizedData::accumulateVariable "accumulateVariable()"
 *
 * \note
 * Accumulation is summation only. No averaging or normalization is performed by this class.
 */
class GDigitizedData : public GBase<GDigitizedData>
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
	GDigitizedData(const std::shared_ptr<GOptions>& gopts, const GHit* ghit);

	/**
	 * \brief Stores or overwrites one integer observable for this hit.
	 *
	 * \details
	 * This method is usually used during event-level filling.
	 * Repeated insertion with the same key replaces the previous value.
	 *
	 * \param vname Observable key.
	 * \param value Integer value to store.
	 */
	void includeVariable(const std::string& vname, int value);

	/**
	 * \brief Stores or overwrites one floating-point observable for this hit.
	 *
	 * \details
	 * This method is usually used during event-level filling.
	 * Repeated insertion with the same key replaces the previous value.
	 *
	 * \param vname Observable key.
	 * \param value Floating-point value to store.
	 */
	void includeVariable(const std::string& vname, double value);

	/**
	 * \brief Accumulates one integer observable into the current object.
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
	void accumulateVariable(const std::string& vname, int value);

	/**
	 * \brief Accumulates one floating-point observable into the current object.
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
	 * \brief Returns a filtered copy of the integer observables map.
	 *
	 * \details
	 * The \p which argument selects the filtering mode:
	 * - \c 0 : return non-SRO variables only
	 * - \c 1 : return SRO variables only
	 *
	 * SRO variables are the conventional readout keys defined in gdataConventions.h.
	 *
	 * \param which Filter selector.
	 * \return Filtered copy of the integer observables.
	 */
	[[nodiscard]] std::map<std::string, int> getIntObservablesMap(int which) const;

	/**
	 * \brief Returns a filtered copy of the floating-point observables map.
	 *
	 * \details
	 * The \p which argument uses the same semantics as
	 * \ref GDigitizedData::getIntObservablesMap "getIntObservablesMap()".
	 *
	 * \param which Filter selector.
	 * \return Filtered copy of the floating-point observables.
	 */
	[[nodiscard]] std::map<std::string, double> getDblObservablesMap(int which) const;

	/**
	 * \brief Returns the conventional \c timeAtElectronics integer observable when present.
	 *
	 * \details
	 * If the key is absent, the method returns the sentinel value
	 * \ref TIMEATELECTRONICSNOTDEFINED.
	 *
	 * This method provides a compact accessor for a commonly used SRO quantity without forcing
	 * callers to inspect the observables map directly.
	 *
	 * \return Stored \c timeAtElectronics value, or \c TIMEATELECTRONICSNOTDEFINED if missing.
	 */
	int getTimeAtElectronics();

	/**
	 * \brief Returns one integer observable by key.
	 *
	 * \details
	 * If the key is missing, an error is reported with the stable error code
	 * \ref ERR_VARIABLENOTFOUND.
	 *
	 * \param varName Observable key.
	 * \return Stored integer value.
	 */
	int getIntObservable(const std::string& varName);

	/**
	 * \brief Returns one floating-point observable by key.
	 *
	 * \details
	 * If the key is missing, an error is reported with the stable error code
	 * \ref ERR_VARIABLENOTFOUND.
	 *
	 * \param varName Observable key.
	 * \return Stored floating-point value.
	 */
	double getDblObservable(const std::string& varName);

	/**
	 * \brief Returns a copy of the array-valued integer observables.
	 *
	 * \details
	 * These arrays are producer-defined and may represent multi-sample or structured readout content.
	 *
	 * \return Copy of the integer-array observables map.
	 */
	[[nodiscard]] inline std::map<std::string, std::vector<int>> getArrayIntObservablesMap() const {
		return arrayIntObservablesMap;
	}

	/**
	 * \brief Returns a copy of the array-valued floating-point observables.
	 *
	 * \details
	 * No SRO filtering is applied to array-valued content.
	 * Interpretation remains detector- or producer-specific.
	 *
	 * \return Copy of the floating-point-array observables map.
	 */
	[[nodiscard]] inline std::map<std::string, std::vector<double>> getArrayDblObservablesMap() const {
		return arrayDoubleObservablesMap;
	}

	/**
	 *
	 * \brief Returns the identifier vectory
	 *
	 * @return gidentity
	 */
	[[nodiscard]] inline const std::vector<GIdentifier>& getIdentity() const { return gidentity; }

	/**
	 * \brief Creates deterministic example data for tests and examples.
	 *
	 * \details
	 * This helper is intended only for demonstrations and tests.
	 * It does not model real detector digitization.
	 *
	 * The returned object contains:
	 * - the conventional SRO keys \c crate, \c slot, \c channel, and \c timeAtElectronics
	 * - one non-SRO floating observable, \c adc
	 *
	 * Uniqueness across calls is derived from a static atomic counter.
	 *
	 * \param gopts Shared options.
	 * \return Newly created example object.
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
	/// Scalar integer observables associated with this digitized hit.
	std::map<std::string, int> intObservablesMap;

	/// Scalar floating-point observables associated with this digitized hit.
	std::map<std::string, double> doubleObservablesMap;

	/// Optional array-valued integer observables.
	std::map<std::string, std::vector<int>> arrayIntObservablesMap;

	/// Optional array-valued floating-point observables.
	std::map<std::string, std::vector<double>> arrayDoubleObservablesMap;

	/// Detector identity copied from the originating hit.
	std::vector<GIdentifier> gidentity;

	/**
	 * \brief Returns whether a key belongs in the requested filtered view.
	 *
	 * \details
	 * Filtering modes:
	 * - \c which = 0 : accept non-SRO keys only
	 * - \c which = 1 : accept SRO keys only
	 *
	 * The check uses the conventional key names defined in gdataConventions.h.
	 *
	 * \param varName Observable key to classify.
	 * \param which   Requested filter selector.
	 * \return \c true if the variable should be included in the selected view.
	 */
	[[nodiscard]] static bool validVarName(const std::string& varName, int which);

	/**
	 * \brief Global example/test counter used by \ref GDigitizedData::create "create()".
	 *
	 * \details
	 * This counter is only used to generate deterministic dummy content in examples and tests.
	 */
	static std::atomic<int> globalDigitizedDataCounter;

protected:
	friend std::ostream& operator<<(std::ostream& os, const GDigitizedData& data);
};