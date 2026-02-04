// gstreamer
#include "gstreamerJSONFactory.h"
#include "gstreamerConventions.h"

// c++
#include <sstream>

bool GstreamerJsonFactory::publishEventTrueInfoDataImpl(const std::string& detectorName,
                                                        const std::vector<const GTrueInfoData*>& trueInfoData) {
	if (!is_building_event) {
		log->error(ERR_PUBLISH_ERROR, "publishEventTrueInfoDataImpl called without an active event in GstreamerJsonFactory");
		return false;
	}

	// Lazily open the detectors object the first time we receive detector data.
	if (!current_event_has_any_detector) {
		current_event << ", \"detectors\": {";
		current_event_has_any_detector = true;
	} else {
		// This function is called once per detector; separator between detector blocks
		// is handled by checking whether the detector already exists is not possible here
		// without tracking. Instead, we always append blocks as they arrive and rely on
		// the gstreamer core ordering: each detector appears once per event.
		current_event << ", ";
	}

	// Create detector object with "true_info" and a placeholder for "digitized".
	current_event << "\"" << jsonEscape(detectorName) << "\": {";

	// True info array
	current_event << "\"true_info\": [";

	bool wrote_first_hit = false;
	for (const auto* hit : trueInfoData) {
		if (!hit) continue;

		if (wrote_first_hit) current_event << ", ";
		wrote_first_hit = true;

		current_event << "{";

		// Address / identity string
		const std::string addr = hit->getIdentityString();
		current_event << "\"address\": \"" << jsonEscape(addr) << "\"";

		// Variables object
		current_event << ", \"vars\": {";

		bool wrote_first_var = false;

		// Double variables
		for (const auto& [name, value] : hit->getDoubleVariablesMap()) {
			if (wrote_first_var) current_event << ", ";
			wrote_first_var = true;
			current_event << "\"" << jsonEscape(name) << "\": " << value;
		}

		// String variables
		for (const auto& [name, value] : hit->getStringVariablesMap()) {
			if (wrote_first_var) current_event << ", ";
			wrote_first_var = true;
			current_event << "\"" << jsonEscape(name) << "\": \"" << jsonEscape(value) << "\"";
		}

		current_event << "}"; // close vars
		current_event << "}"; // close hit
	}

	current_event << "]"; // close true_info array

	// Digitized array will be appended by publishEventDigitizedDataImpl for the same detector.
	current_event << ", \"digitized\": ";

	// We do not know digitized content here; a sentinel is written and replaced immediately
	// by the next call because the core calls digitized publishing right after true info.
	// To avoid string replacement complexity, we write an empty array here and let
	// publishEventDigitizedDataImpl append a second "digitized" field ONLY if it has data.
	//
	// Practically: keep the schema stable even if digitized is empty.
	current_event << "[]";

	current_event << "}"; // close detector object

	return true;
}
