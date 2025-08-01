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
#include "glogger.h"

class GTrueInfoData {
public:
	/**
	 * \brief Constructs a GTrueInfoData object.
	 * \param ghit Pointer to the GHit from which identity information is extracted.
	 * \param logger GLogger Shared pointer
	 */
	GTrueInfoData(const GHit* ghit, std::shared_ptr<GLogger> logger);

	/**
	 * \brief Destructor for GTrueInfoData.
	 */
	~GTrueInfoData() { if (log) log->debug(DESTRUCTOR, "GTrueInfoData"); }

	/**
	 * \brief Returns a string representation of the identity.
	 * \return A string representing the hit identity.
	 */
	std::string getIdentityString() const;

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

	/**
	 * \brief Returns the map of double variables.
	 * \return A map from variable names to double values.
	 */
	[[nodiscard]] inline std::map<std::string, double> getDoubleVariablesMap() const {
		return trueInfoDoublesVariablesMap;
	}

	/**
	 * \brief Returns the map of string variables.
	 * \return A map from variable names to string values.
	 */
	[[nodiscard]] inline std::map<std::string, std::string> getStringVariablesMap() const {
		return trueInfoStringVariablesMap;
	}

	static std::unique_ptr<GTrueInfoData> create(std::shared_ptr<GLogger> logger) {
		auto hit       = GHit::create(logger);
		auto true_info_data = std::make_unique<GTrueInfoData>(hit, logger);
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
	std::map<std::string, double>      trueInfoDoublesVariablesMap; ///< Map of double variables.
	std::map<std::string, std::string> trueInfoStringVariablesMap;  ///< Map of string variables.
	std::vector<GIdentifier>           gidentity;                   ///< Vector of identifiers extracted from the hit.
	std::shared_ptr<GLogger>           log;                         ///< Logger instance

	/// Static thread-safe event counter - used for testing only
	static std::atomic<int> globalTrueInfoDataCounter;
};

