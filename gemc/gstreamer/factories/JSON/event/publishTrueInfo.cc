// gstreamer
#include "gstreamerJSONFactory.h"
#include "gstreamerConventions.h"

// c++
#include <sstream>

// Implementation summary:
// Append one detector true-information block to the current JSON event object.

bool GstreamerJsonFactory::publishEventTrueInfoDataImpl(const std::string& detectorName,
                                                        const std::vector<const GTrueInfoData*>& trueInfoData) {
	if (!is_building_event) {
		log->error(ERR_PUBLISH_ERROR, "publishEventTrueInfoDataImpl called without an active event in GstreamerJsonFactory");
		return false;
	}

	// Lazily open the "detectors" object on the first detector encountered.
	if (!current_event_has_any_detector) {
		current_event << ", \"detectors\": {";
		current_event_has_any_detector = true;
	} else {
		current_event << ", ";
	}

	current_event << "\"" << jsonEscape(detectorName) << "\": {";

	// True-information hits are serialized as an array of objects.
	current_event << "\"true_info\": [";

	bool wrote_first_hit = false;
	for (const auto* hit : trueInfoData) {
		if (!hit) continue;

		if (wrote_first_hit) current_event << ", ";
		wrote_first_hit = true;

		current_event << "{";

		auto addr = getIdentityString(hit->getIdentity());

		current_event << "\"address\": \"" << jsonEscape(addr) << "\"";

		current_event << ", \"vars\": {";

		bool wrote_first_var = false;

		for (const auto& [name, value] : hit->getDoubleVariablesMap()) {
			if (wrote_first_var) current_event << ", ";
			wrote_first_var = true;
			current_event << "\"" << jsonEscape(name) << "\": " << value;
		}

		for (const auto& [name, value] : hit->getStringVariablesMap()) {
			if (wrote_first_var) current_event << ", ";
			wrote_first_var = true;
			current_event << "\"" << jsonEscape(name) << "\": \"" << jsonEscape(value) << "\"";
		}

		current_event << "}";
		current_event << "}";
	}

	current_event << "]";

	// Keep a stable schema even before digitized content is appended for this detector.
	current_event << ", \"digitized\": ";
	current_event << "[]";

	current_event << "}";

	return true;
}