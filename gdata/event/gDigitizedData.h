#ifndef GDIGITIZEDHIT_H
#define GDIGITIZEDHIT_H 1

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
	GDigitizedData(GHit *ghit, GLogger * const logger);

	/**
	 * \brief Destructor for GDigitizedData.
	 */
	~GDigitizedData() {
		log->debug(DESTRUCTOR, "GDigitizedData");
	}

	/**
	 * \brief Returns a string representation of the hit identity.
	 * \return A string representing the identity.
	 */
	std::string getIdentityString();

	// Public interface to add data to a hit:
	void includeVariable(std::string vname, int value);
	void includeVariable(std::string vname, float value);
	void includeVariable(std::string vname, std::vector<int> values);
	void includeVariable(std::string vname, std::vector<float> values);

	/**
	 * \brief Returns the filtered map of integer observables.
	 * \param which 0: returns non-streaming variables; 1: returns streaming variables.
	 * \return A map of variable names to integer values.
	 */
	const std::map<std::string, int> getIntObservablesMap(int which) const;

	/**
	 * \brief Returns the filtered map of float observables.
	 * \param which 0: returns non-streaming variables; 1: returns streaming variables.
	 * \return A map of variable names to float values.
	 */
	const std::map<std::string, float> getFltObservablesMap(int which) const;

	/**
	 * \brief Gets the time at electronics.
	 *
	 * Returns -1 if TIMEATELECTRONICS is not included.
	 *
	 * \return The time at electronics.
	 */
	int getTimeAtElectronics();

	int getIntObservable(std::string varName);
	float getFltObservable(std::string varName);

	/**
	 * \brief Returns the map of integer array observables.
	 * \return A map of variable names to vectors of integers.
	 */
	inline const std::map<std::string, std::vector<int>> getArrayIntObservablesMap() const { return arrayIntObservablesMap; }

	/**
	 * \brief Returns the map of float array observables.
	 * \return A map of variable names to vectors of floats.
	 */
	inline const std::map<std::string, std::vector<float>> getArrayFltObservablesMap() const { return arrayFltObservablesMap; }

private:
	std::map<std::string, int> intObservablesMap;    ///< Map of integer observables.
	std::map<std::string, float> fltObservablesMap;    ///< Map of float observables.
	std::map<std::string, std::vector<int>> arrayIntObservablesMap;   ///< Map of integer array observables.
	std::map<std::string, std::vector<float>> arrayFltObservablesMap; ///< Map of float array observables.
	std::vector<GIdentifier> gidentity;  ///< Identity extracted from the hit.
	bool validVarName(std::string varName, int which) const; ///< Validates variable names.
	GLogger * const log; ///< Logger instance.
};

#endif
