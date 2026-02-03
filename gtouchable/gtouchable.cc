// gtouchable
#include "gtouchable.h"
#include "gtouchableConventions.h"

// gemc
#include "gutilities.h"
#include "gutsConventions.h"

// Define the static counter here
std::atomic<int> GTouchable::globalGTouchableCounter{0};

// See header for API docs.

// constructor from gopt, digitization and gidentity strings
// called in GDetectorConstruction::ConstructSDandField
GTouchable::GTouchable(const std::shared_ptr<GOptions>& gopt,
                       const std::string&               digitization,
                       const std::string&               gidentityString,
                       const std::vector<double>&       dimensions) : GBase(gopt, TOUCHABLE_LOGGER),
                                                                      trackId(0),
                                                                      eMultiplier(1),
                                                                      stepTimeAtElectronicsIndex(
	                                                                      GTOUCHABLEUNSETTIMEINDEX),
                                                                      detectorDimensions(dimensions) {
	// Determine the type based on the digitization string.
	// The string constants are defined in gtouchableConventions.h : FLUXNAME, COUNTERNAME, DOSIMETERNAME.
	if (digitization == FLUXNAME) { gType = flux; }
	else if (digitization == COUNTERNAME) { gType = particleCounter; }
	else if (digitization == DOSIMETERNAME) { gType = dosimeter; }
	else { gType = readout; }

	// Parse the gidentity string into (name,value) pairs.
	// Expected format: "sector: 2, layer: 4, wire: 33"
	// The identity vector order is preserved because comparisons assume the same schema/order.
	std::vector<std::string> identity = gutilities::getStringVectorFromStringWithDelimiter(gidentityString, ",");
	// Process each identifier token (e.g., "sector: 2").
	for (auto& gid : identity) {
		std::vector<std::string> identifier = gutilities::getStringVectorFromStringWithDelimiter(gid, ":");

		// The parser assumes the token splits into [name, value].
		// Note: In production, consider adding try-catch here to handle conversion errors.
		const std::string& idName  = identifier[0];
		int                idValue = std::stoi(identifier[1]);

		gidentity.emplace_back(idName, idValue);
	}
	log->debug(CONSTRUCTOR, "GTouchable", gtouchable::to_string(gType), " ", getIdentityString());
}

// constructor from logger, digitization and gidentity strings
// called in GDetectorConstruction::ConstructSDandField
GTouchable::GTouchable(const std::shared_ptr<GLogger>& logger,
                       const std::string&              digitization,
                       const std::string&              gidentityString,
                       const std::vector<double>&      dimensions) : GBase(logger),
                                                                     trackId(0),
                                                                     eMultiplier(1),
                                                                     stepTimeAtElectronicsIndex(
	                                                                     GTOUCHABLEUNSETTIMEINDEX),
                                                                     detectorDimensions(dimensions) {
	// Determine the type based on the digitization string.
	// The string constants are defined in gtouchableConventions.h : FLUXNAME, COUNTERNAME, DOSIMETERNAME.
	if (digitization == FLUXNAME) { gType = flux; }
	else if (digitization == COUNTERNAME) { gType = particleCounter; }
	else if (digitization == DOSIMETERNAME) { gType = dosimeter; }
	else { gType = readout; }

	// Parse the gidentity string into (name,value) pairs.
	// Expected format: "sector: 2, layer: 4, wire: 33"
	// The identity vector order is preserved because comparisons assume the same schema/order.
	std::vector<std::string> identity = gutilities::getStringVectorFromStringWithDelimiter(gidentityString, ",");
	// Process each identifier token (e.g., "sector: 2").
	for (auto& gid : identity) {
		std::vector<std::string> identifier = gutilities::getStringVectorFromStringWithDelimiter(gid, ":");

		// The parser assumes the token splits into [name, value].
		// Note: In production, consider adding try-catch here to handle conversion errors.
		const std::string& idName  = identifier[0];
		int                idValue = std::stoi(identifier[1]);

		gidentity.emplace_back(idName, idValue);
	}
	log->debug(CONSTRUCTOR, "GTouchable", gtouchable::to_string(gType), " ", getIdentityString());
}

// Overloaded "==" operator for the class 'GTouchable'
bool GTouchable::operator==(const GTouchable& that) const {
	// First, check if both gidentity vectors are the same size.
	// this should never happen because the same sensitivity should be assigned the same identifier structure
	if (this->gidentity.size() != that.gidentity.size()) {
		log->debug(NORMAL, "Touchable sizes are different");
		return false;
	}

	// Compare identifiers positionally.
	// Only the identifier values are compared (schema/order is assumed identical for the same sensitivity).
	log->debug(NORMAL, "  + Touchable comparison:  ");
	for (size_t i = 0; i < this->gidentity.size(); ++i) {
		bool        equal            = (this->gidentity[i].getValue() == that.gidentity[i].getValue());
		std::string comparisonResult = equal ? " ✅" : " ❌";
		log->debug(NORMAL, "     ← ", this->gidentity[i], "   → ", that.gidentity[i], comparisonResult);
		if (!equal) { return false; }
	}

	bool        typeComparison = false;
	std::string result;

	// All identity values matched; apply the type-specific discriminator.
	switch (this->gType) {
	case readout:
		typeComparison = this->stepTimeAtElectronicsIndex == that.stepTimeAtElectronicsIndex;
		result = typeComparison ? " ✅" : " ❌";
		log->debug(NORMAL, "    Touchable type is readout. Time cell comparison: ", this->stepTimeAtElectronicsIndex,
		           " ", that.stepTimeAtElectronicsIndex,
		           " result:", result);
		break;
	case flux:
		typeComparison = this->trackId == that.trackId;
		result = typeComparison ? " ✅" : " ❌";
		log->debug(NORMAL, "    Touchable type is flux. Track id comparison: ", this->trackId, " ", that.trackId,
		           " result:", result);
		break;
	case dosimeter:
		typeComparison = this->trackId == that.trackId;
		result = typeComparison ? " ✅" : " ❌";
		log->debug(NORMAL, "    Touchable type is dosimeter. Track id comparison: ", this->trackId, " ", that.trackId,
		           " result:", result);
		break;
	case particleCounter:
		typeComparison = true;
		log->debug(NORMAL, "    Touchable type is particleCounter. No additional comparison needed, returning true  ✅");
		break;
	}
	return typeComparison;
}

// ostream GTouchable
std::ostream& operator<<(std::ostream& stream, const GTouchable& gtouchable) {
	stream << " GTouchable: ";
	for (auto& gid : gtouchable.gidentity) {
		stream << KRED << gid;
		if (gid.getName() != gtouchable.gidentity.back().getName()) { stream << ", "; }
		else { stream << RST; }
	}
	switch (gtouchable.gType) {
	case readout:
		// compare the time cell
		stream << KGRN << " (readout), " << RST << " multiplier: " << gtouchable.eMultiplier << ", time cell index: " <<
			gtouchable.stepTimeAtElectronicsIndex;
		break;
	case flux:
		stream << KGRN << " (flux), " << RST << " g4 track id: " << gtouchable.trackId;
		break;
	case dosimeter:
		stream << KGRN << " (dosimeter), " << RST << " g4 track id: " << gtouchable.trackId;
		break;
	case particleCounter:
		stream << KGRN << " (particleCounter), " << RST << " g4 track id: " << gtouchable.trackId;
		break;
	}

	return stream;
}

/// Overloaded output operator for GIdentifier.
std::ostream& operator<<(std::ostream& stream, const GIdentifier& gidentifier) {
	stream << gidentifier.idName << ": " << gidentifier.idValue;
	return stream;
}
