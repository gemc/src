#pragma once

/**
 * \file GDigitizedData.h
 * \brief Defines the GDigitizedData class for storing digitized hit data.
 *
 * This class stores observables produced during digitization.
 * It maintains maps for single-value and array observables keyed by variable names.
 */

// c++
#include <string>
#include <map>
#include <vector>
#include <atomic>

// gemc
#include "ghit.h"
#include "glogger.h"
#include "gdataConventions.h"

class GDigitizedData {
public:
	/**
	 * \brief Constructs a GDigitizedData object from a GHit.
	 * \param ghit unique_ptr GHit from which identity information is extracted.
	 * \param logger Pointer to a GLogger instance.
	 */
	GDigitizedData(const GHit* ghit, const std::shared_ptr<GLogger>& logger);

	/**
	 * \brief Destructor for GDigitizedData.
	 */
	~GDigitizedData() { log->debug(DESTRUCTOR, "GDigitizedData"); }

	/**
	 * \brief Returns a string representation of the hit identity.
	 * \return A string representing the identity.
	 */
	std::string getIdentityString() const;

	// Public interface to add data to a hit:
	void includeVariable(const std::string& vname, int value);
	void includeVariable(const std::string& vname, double value);
	// void includeVariable(std::string vname, std::vector<int> values);

	/**
	 * \brief Returns the filtered map of integer observables.
	 * \param which 0: returns non-streaming variables; 1: returns streaming variables.
	 * \return A map of variable names to integer values.
	 */
	[[nodiscard]] std::map<std::string, int> getIntObservablesMap(int which) const;

	/**
	 * \brief Returns the filtered map of double observables.
	 * \param which 0: returns non-streaming variables; 1: returns streaming variables.
	 * \return A map of variable names to double values.
	 */
	[[nodiscard]] std::map<std::string, double> getDblObservablesMap(int which) const;

	/**
	 * \brief Gets the time at electronics.
	 *
	 * Returns -1 if TIMEATELECTRONICS is not included.
	 *
	 * \return The time at electronics.
	 */
	int getTimeAtElectronics();

	int    getIntObservable(const std::string& varName);
	double getDblObservable(const std::string& varName);

	/**
	 * \brief Returns the map of integer array observables.
	 * \return A map of variable names to vectors of integers.
	 */
	[[nodiscard]] inline std::map<std::string, std::vector<int>> getArrayIntObservablesMap() const { return arrayIntObservablesMap; }

	/**
	 * \brief Returns the map of double array observables.
	 * \return A map of variable names to vectors of double.
	 */
	[[nodiscard]] inline std::map<std::string, std::vector<double>> getArrayDblObservablesMap() const { return arrayDoubleObservablesMap; }


	static std::unique_ptr<GDigitizedData> create(std::shared_ptr<GLogger> logger) {
		auto hit       = GHit::create(logger);
		auto digi_data = std::make_unique<GDigitizedData>(hit, logger);
		auto counter   = globalDigitizedDataCounter.fetch_add(1, std::memory_order_relaxed);

		digi_data->includeVariable(CRATESTRINGID, counter % 10);
		digi_data->includeVariable(SLOTSTRINGID, counter % 20);
		digi_data->includeVariable(CHANNELSTRINGID, counter);
		digi_data->includeVariable(TIMEATELECTRONICS, counter * 5);
		digi_data->includeVariable("adc", counter * 0.1);
		return digi_data;
	}

private:
	std::map<std::string, int>                 intObservablesMap;                                   ///< Map of integer observables.
	std::map<std::string, double>              doubleObservablesMap;                                ///< Map of double observables.
	std::map<std::string, std::vector<int>>    arrayIntObservablesMap;                              ///< Map of integer array observables.
	std::map<std::string, std::vector<double>> arrayDoubleObservablesMap;                           ///< Map of double array observables.
	std::vector<GIdentifier>                   gidentity;                                           ///< Identity extracted from the hit.
	[[nodiscard]] static bool                  validVarName(const std::string& varName, int which); ///< Validates variable names.
	std::shared_ptr<GLogger>                   log;                                                 ///< Logger instance

	/// Static thread-safe event counter - used for testing only
	static std::atomic<int> globalDigitizedDataCounter;
};
