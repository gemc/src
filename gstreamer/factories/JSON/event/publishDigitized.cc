// gstreamer
#include "gstreamerJSONFactory.h"
#include "gstreamerConventions.h"

// c++
#include <sstream>

// Implementation summary:
// Append digitized detector content to the current JSON event object.
// This implementation keeps the JSON valid without performing in-place string editing.

bool GstreamerJsonFactory::publishEventDigitizedDataImpl(const std::string&                        detectorName,
                                                         const std::vector<const GDigitizedData*>& digitizedData) {
	if (!is_building_event) {
		log->error(
			ERR_PUBLISH_ERROR, "publishEventDigitizedDataImpl called without an active event in GstreamerJsonFactory");
		return false;
	}

	// The current design appends digitized content into a reserved object nested under
	// "detectors". This avoids rewriting previously emitted detector JSON.
	if (digitizedData.empty()) return true;

	static const char* marker    = "\"digitized_by_detector\": {";
	const std::string  assembled = current_event.str();

	const bool has_digitized_container = (assembled.find(marker) != std::string::npos);

	if (!has_digitized_container) {
		// Ensure the event already has a detectors object before reserving a nested map
		// for digitized collections keyed by detector name.
		if (!current_event_has_any_detector) {
			current_event << ", \"detectors\": {";
			current_event_has_any_detector = true;
		}
		else {
			current_event << ", ";
		}

		current_event << "\"digitized_by_detector\": {";
	}

	// If the reserved object already contains one detector entry, append a comma
	// before adding the next one.
	const std::string updated = current_event.str();
	if (!updated.empty()) {
		char last = updated.back();
		if (last != '{') current_event << ", ";
	}

	current_event << "\"" << jsonEscape(detectorName) << "\": [";

	bool wrote_first_hit = false;
	for (const auto* hit : digitizedData) {
		if (!hit) continue;

		if (wrote_first_hit) current_event << ", ";
		wrote_first_hit = true;

		current_event << "{";

		const std::string addr = hit->getIdentityString();
		current_event << "\"address\": \"" << jsonEscape(addr) << "\"";

		current_event << ", \"vars\": {";

		bool wrote_first_var = false;

		// Integer observables:
		// the argument 0 means "do not include SRO variables".
		for (const auto& [name, value] : hit->getIntObservablesMap(0)) {
			if (wrote_first_var) current_event << ", ";
			wrote_first_var = true;
			current_event << "\"" << jsonEscape(name) << "\": " << value;
		}

		// Floating-point observables.
		for (const auto& [name, value] : hit->getDblObservablesMap(0)) {
			if (wrote_first_var) current_event << ", ";
			wrote_first_var = true;
			current_event << "\"" << jsonEscape(name) << "\": " << value;
		}

		current_event << "}";
		current_event << "}";
	}

	current_event << "]";

	// Close the temporary digitized_by_detector object immediately so the enclosing
	// event remains structurally valid after this call.
	current_event << "}";

	return true;
}