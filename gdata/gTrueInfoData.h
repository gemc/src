#pragma once

/**
 * \file gTrueInfoData.h
 * \brief Defines the GTrueInfoData class which holds true hit data.
 *
 * The GTrueInfoData class stores information for a single hit, including variables
 * and identity information extracted from a GHit.
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
inline GOptions defineOptions() {
	auto goptions = GOptions(GTRUEDATA_LOGGER);
	return goptions;
}
}


class GTrueInfoData : public GBase<GTrueInfoData> {
public:
	/**
	 * \brief Constructs a GTrueInfoData object.
	 * \param ghit Pointer to the GHit from which identity information is extracted.
	 * \param gopts GOptions Shared pointer
	 */
	GTrueInfoData(const std::shared_ptr<GOptions>& gopts, const GHit* ghit);

	/**
	 * \brief Returns a string representation of the identity.
	 * \return A string representing the hit identity.
	 */
	[[nodiscard]] std::string getIdentityString() const;

	/**
	 * \brief Includes a double variable in the true hit data.
	 * \param varName The variable name.
	 * \param var The double value.
	 */
	void includeVariable(const std::string& varName, double var);

	/**
	 * \brief Includes a string variable in the true hit data.
	 * \param varName The variable name.
	 * \param var The string value.
	 */
	void includeVariable(const std::string& varName, std::string var);

	// Public interface to accumulate data:
	void accumulateVariable(const std::string& vname, double value);
	void accumulateVariable(const std::string& vname, std::string var);

	/**
	 * \brief Returns the map of double variables.
	 * \return A map from variable names to double values.
	 */
	[[nodiscard]] inline std::map<std::string, double> getDoubleVariablesMap() const {
		return doubleObservablesMap;
	}

	/**
	 * \brief Returns the map of string variables.
	 * \return A map from variable names to string values.
	 */
	[[nodiscard]] inline std::map<std::string, std::string> getStringVariablesMap() const {
		return stringVariablesMap;
	}

	static std::unique_ptr<GTrueInfoData> create(const std::shared_ptr<GOptions>& gopts) {
		auto hit       = GHit::create(gopts);
		auto true_info_data = std::make_unique<GTrueInfoData>(gopts, hit);
		auto counter   = globalTrueInfoDataCounter.fetch_add(1, std::memory_order_relaxed);

		true_info_data->includeVariable("totalEDeposited", counter * 0.1);
		true_info_data->includeVariable("avgTime", counter * 1.0);
		true_info_data->includeVariable("avgx", counter * 0.01);
		true_info_data->includeVariable("avgy", counter * 0.02);
		true_info_data->includeVariable("avgz", counter * 0.03);
		true_info_data->includeVariable("hitn", counter);

		return true_info_data;
	}

private:
	std::map<std::string, double>      doubleObservablesMap; ///< Map of double variables.
	std::map<std::string, std::string> stringVariablesMap;  ///< Map of string variables.
	std::vector<GIdentifier>           gidentity;                   ///< Vector of identifiers extracted from the hit.

	/// Static thread-safe event counter - used for testing only
	static std::atomic<int> globalTrueInfoDataCounter;
};

