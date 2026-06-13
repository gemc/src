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

	// Build this detector's digitized array into a standalone entry and buffer it.
	// endEventImpl emits all buffered entries together as the "digitized_by_detector"
	// object, which keeps the JSON valid regardless of how true-info and digitized
	// publish calls interleave across detectors.
	if (digitizedData.empty()) return true;

	std::ostringstream entry;
	entry << "\"" << jsonEscape(detectorName) << "\": [";

	bool wrote_first_hit = false;
	for (const auto* hit : digitizedData) {
		if (!hit) continue;

		if (wrote_first_hit) entry << ", ";
		wrote_first_hit = true;

		entry << "{";

		auto addr = getIdentityString(hit->getIdentity());

		entry << "\"address\": \"" << jsonEscape(addr) << "\"";

		entry << ", \"vars\": {";

		bool wrote_first_var = false;

		// Integer observables:
		// the argument 0 means "do not include SRO variables".
		for (const auto& [name, value] : hit->getIntObservablesMap(0)) {
			if (wrote_first_var) entry << ", ";
			wrote_first_var = true;
			entry << "\"" << jsonEscape(name) << "\": " << value;
		}

		// Floating-point observables.
		for (const auto& [name, value] : hit->getDblObservablesMap(0)) {
			if (wrote_first_var) entry << ", ";
			wrote_first_var = true;
			entry << "\"" << jsonEscape(name) << "\": " << value;
		}

		entry << "}";
		entry << "}";
	}

	entry << "]";

	current_event_digitized_entries.push_back(entry.str());

	return true;
}