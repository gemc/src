#pragma once

// gemc
#include "gelectronic.h"
#include "gbase.h"

// c++
#include <string>
#include <unordered_map>
#include <vector>

// tt
#include "gtranslationTable_options.h"

/**
 * \class GTranslationTable
 * \brief Provides a translation table for GElectronic objects indexed by a vector-based identity.
 *
 * This class maps a vector<int> identity (converted into a string key) to a GElectronic object.
 */
class GTranslationTable : public GBase<GTranslationTable> {

public:
	/**
	 * \brief Constructor.
	 * \param gopt Pointer to GOptions
	 */
	explicit GTranslationTable(const std::shared_ptr<GOptions>& gopt) : GBase(gopt, TRANSLATIONTABLE_LOGGER) {  }


	/**
	 * \brief Adds a GElectronic configuration with the provided identity.
	 *
	 * If the identity already exists in the table, a warning is logged.
	 *
	 * \param identity A vector of integers representing the unique identity.
	 * \param gtron The GElectronic object to be added.
	 */
	void addGElectronicWithIdentity(const std::vector<int>& identity, const GElectronic& gtron);

	/**
	 * \brief Retrieves the GElectronic configuration for the provided identity.
	 *
	 * If the identity is not found, an error is logged and a default GElectronic is returned.
	 *
	 * \param identity A vector of integers representing the unique identity.
	 * \return The GElectronic object corresponding to the identity.
	 */
	[[nodiscard]] GElectronic getElectronics(const std::vector<int>& identity) const;

private:
	// the map uses a string formed by the vector<int> identity as its key
	std::unordered_map<std::string, GElectronic> tt;

	/**
	 * \brief Forms the key for the translation table from an identity vector.
	 *
	 * The key is formed by concatenating the integer values separated by a hyphen.
	 *
	 * \param identity A vector of integers representing the identity.
	 * \return A string key representing the identity.
	 */
	[[nodiscard]] std::string formTTKey(const std::vector<int>& identity) const;

};
