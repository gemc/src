#pragma once

/**
 * \file GDigitizedData.h
 * \brief Defines the GDigitizedData class for storing digitized hit data.
 *
 * This class stores observables produced during digitization.
 * It maintains maps for single-value and array observables keyed by variable names.
 */

#include <string>
#include <map>
#include <vector>
#include "ghit.h"
#include "glogger.h"

class GDigitizedData {
public:
	/**
	 * \brief Constructs a GDigitizedData object from a GHit.
	 * \param ghit Pointer to the GHit from which identity information is extracted.
	 * \param logger Pointer to a GLogger instance.
	 */
	GDigitizedData(GHit* ghit, const std::shared_ptr<GLogger>& logger);

	/**
	 * \brief Destructor for GDigitizedData.
	 */
	~GDigitizedData() { log->debug(DESTRUCTOR, "GDigitizedData"); }

	/**
	 * \brief Returns a string representation of the hit identity.
	 * \return A string representing the identity.
	 */
	std::string getIdentityString();

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

private:
	std::map<std::string, int>                 intObservablesMap;                                   ///< Map of integer observables.
	std::map<std::string, double>              doubleObservablesMap;                                ///< Map of double observables.
	std::map<std::string, std::vector<int>>    arrayIntObservablesMap;                              ///< Map of integer array observables.
	std::map<std::string, std::vector<double>> arrayDoubleObservablesMap;                           ///< Map of double array observables.
	std::vector<GIdentifier>                   gidentity;                                           ///< Identity extracted from the hit.
	[[nodiscard]] static bool                  validVarName(const std::string& varName, int which); ///< Validates variable names.
	std::shared_ptr<GLogger>                   log;                                                 ///< Logger instance
};
