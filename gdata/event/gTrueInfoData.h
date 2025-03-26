#ifndef GTRUEINFOHIT_H
#define GTRUEINFOHIT_H 1

/**
 * \file gTrueInfoData.h
 * \brief Defines the GTrueInfoData class which holds true hit data.
 *
 * The GTrueInfoData class stores information for a single hit, including variables
 * and identity information extracted from a GHit.
 */

#include <string>
#include <map>
#include <vector>
#include "ghit.h"
#include "glogger.h"

class GTrueInfoData {
public:
	/**
	 * \brief Constructs a GTrueInfoData object.
	 * \param ghit Pointer to the GHit from which identity information is extracted.
	 * \param logger Pointer to a GLogger instance used for logging.
	 */
	GTrueInfoData(GHit *ghit, GLogger * const logger);

	/**
	 * \brief Destructor for GTrueInfoData.
	 */
	~GTrueInfoData() {
		log->debug(DESTRUCTOR, "GTrueInfoData");
	}

	/**
	 * \brief Returns a string representation of the identity.
	 * \return A string representing the hit identity.
	 */
	std::string getIdentityString();

	/**
	 * \brief Includes a float variable in the true hit data.
	 * \param varName The variable name.
	 * \param var The float value.
	 */
	void includeVariable(std::string varName, float var);

	/**
	 * \brief Includes a string variable in the true hit data.
	 * \param varName The variable name.
	 * \param var The string value.
	 */
	void includeVariable(std::string varName, std::string var);

	/**
	 * \brief Returns the map of float variables.
	 * \return A map from variable names to float values.
	 */
	inline const std::map<std::string, float> getFloatVariablesMap() const { return trueInfoFloatVariablesMap; }

	/**
	 * \brief Returns the map of string variables.
	 * \return A map from variable names to string values.
	 */
	inline const std::map<std::string, std::string> getStringVariablesMap() const { return trueInfoStringVariablesMap; }

private:
	std::map<std::string, float>  trueInfoFloatVariablesMap;  ///< Map of float variables.
	std::map<std::string, std::string> trueInfoStringVariablesMap; ///< Map of string variables.
	std::vector<GIdentifier> gidentity;  ///< Vector of identifiers extracted from the hit.
	GLogger * const log;                 ///< Logger instance.
};

#endif
