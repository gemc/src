// gtranslationTable
#include "gtranslationTable.h"
#include "gtranslationTableConventions.h"


// See header for API docs.
std::string GTranslationTable::formTTKey(const std::vector<int> &identity) const {
	// Defensive check: an empty identity cannot form a meaningful key.
	// Returning an empty key also prevents accidental insertion under an ambiguous identifier.
	if (identity.empty()) {
		log->warning("Empty identity vector provided to formTTKey");
		return "";
	}

	// Build a hyphen-separated key (e.g. "1-2-3-4").
	std::ostringstream oss;
	for (size_t i = 0; i < identity.size(); ++i) {
		oss << identity[i];
		if (i != identity.size() - 1)
			oss << "-";
	}
	return oss.str();
}


// See header for API docs.
void GTranslationTable::addGElectronicWithIdentity(const std::vector<int> &identity, const GElectronic &gtron) {
	std::string ttKey = formTTKey(identity);
	const bool inserted = tt.try_emplace(ttKey, gtron).second;

	if (!inserted) {
		log->warning("Key <%1> already present in TT map", ttKey);
	}

	// Level 1: typical "milestone" message indicating a configuration registration was attempted.
	log->info(1, "%1 GElectronic with identity <%2> in TT map", inserted ? "Added" : "Preserved", ttKey);

	// Debug: print the entire table content for troubleshooting configuration/key issues.
	log->debug(NORMAL, "Translation Table:");
	for (auto &thisItem: tt) {
		log->debug(NORMAL, "%1<%2>  ⇢ %3", guts::GTAB, thisItem.first, thisItem.second);
	}
}


// See header for API docs.
const GElectronic& GTranslationTable::getElectronics(const std::vector<int> &identity) const {
	std::string ttKey = formTTKey(identity);
	auto search = tt.find(ttKey);

	if (search == tt.end()) {
		log->error(gtranslationTable::EC__TTNOTFOUNDINTT, "Key <%1> not found in TT map", ttKey);
	}

	log->debug(NORMAL, "Retrieved Electronic using key <%1> in TT map: %2", ttKey, search->second);
	return search->second;
}
