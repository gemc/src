// gtranslationTable
#include "gtranslationTable.h"
#include "gtranslationTableConventions.h"


/**
 * \brief Forms the translation table key from the given identity vector.
 *
 * Uses an std::ostringstream to concatenate the integer elements separated by hyphens.
 * If the identity vector is empty, logs a warning and returns an empty string.
 *
 * \param identity A vector of integers representing the identity.
 * \return A hyphen-separated string key.
 */
std::string GTranslationTable::formTTKey(const std::vector<int> &identity) const {
	// Check for an empty identity vector to avoid undefined behavior.
	if (identity.empty()) {
		log->warning("Empty identity vector provided to formTTKey");
		return "";
	}

	std::ostringstream oss;
	for (size_t i = 0; i < identity.size(); ++i) {
		oss << identity[i];
		if (i != identity.size() - 1)
			oss << "-";
	}
	return oss.str();
}


/**
 * \brief Adds a GElectronic object to the translation table using the provided identity.
 *
 * If the key already exists, logs a warning instead of overwriting.
 *
 * \param identity A vector of integers representing the unique identity.
 * \param gtron The GElectronic object to be added.
 */
void GTranslationTable::addGElectronicWithIdentity(const std::vector<int> &identity, const GElectronic &gtron) {
	std::string ttKey = formTTKey(identity);
	auto search = tt.find(ttKey);

	if (search == tt.end()) {
		// Insert the new key-value pair
		tt[ttKey] = gtron;
	} else {
		log->warning("Key <" + ttKey + "> already present in TT map");
	}

	log->info(1, "Added GElectronic with identity <", ttKey, "> to TT map");

	log->debug(NORMAL, "Translation Table:");
	for (auto &thisItem: tt) {
		log->debug(NORMAL, GTAB, "<", thisItem.first, ">  ⇢ ", thisItem.second);
	}
}


/**
 * \brief Retrieves the GElectronic object corresponding to the provided identity.
 *
 * Logs an error if the key is not found and returns a default constructed GElectronic.
 *
 * \param identity A vector of integers representing the unique identity.
 * \return The GElectronic object if found; otherwise, a default GElectronic.
 */
GElectronic GTranslationTable::getElectronics(const std::vector<int> &identity) const {
	std::string ttKey = formTTKey(identity);
	auto search = tt.find(ttKey);

	if (search != tt.end()) {
		log->debug(NORMAL, "Found key <", ttKey, "> in TT map");
		return search->second;
	} else {
		log->error(EC__TTNOTFOUNDINTT, "Key <", ttKey, "> not found in TT map");
	}
}

