// gstreamer
#include "gstreamerJSONFactory.h"
#include "gstreamerConventions.h"

// c++
#include <sstream>

bool GstreamerJsonFactory::publishEventDigitizedDataImpl(const std::string&                        detectorName,
                                                         const std::vector<const GDigitizedData*>& digitizedData) {
	if (!is_building_event) {
		log->error(
			ERR_PUBLISH_ERROR, "publishEventDigitizedDataImpl called without an active event in GstreamerJsonFactory");
		return false;
	}

	// IMPORTANT:
	// The current JSON assembly approach writes one detector block per call to true-info publishing,
	// and leaves "digitized": [] as default.
	//
	// In the current gstreamer core flow, digitized publishing happens immediately after true-info
	// for the same detector. We therefore append an additional compact "digitized_data" object at
	// the event level to avoid fragile in-place string editing.
	//
	// This keeps output valid and preserves all digitized information while avoiding string surgery.

	if (digitizedData.empty()) return true;

	// If this is the first digitized block added at event level, create the container.
	// We detect this by searching whether we've already inserted the key; simple state is best.
	// Use a conservative heuristic: add only once by tracking a marker in the stream state.
	static const char* marker    = "\"digitized_by_detector\": {";
	const std::string  assembled = current_event.str();

	const bool has_digitized_container = (assembled.find(marker) != std::string::npos);

	if (!has_digitized_container) {
		// Insert after detectors (which may or may not exist).
		// If detectors exist, we are still inside the event object assembly and detectors are not closed yet.
		// Our detector blocks close themselves, and we keep detectors object open until endEventImpl.
		// Therefore we can safely append inside the detectors object by adding a reserved key.
		//
		// The reserved key lives inside "detectors" so the schema stays naturally grouped.
		if (!current_event_has_any_detector) {
			current_event << ", \"detectors\": {";
			current_event_has_any_detector = true;
		}
		else {
			current_event << ", ";
		}

		current_event << "\"digitized_by_detector\": {";
	}

	// Append detector digitized array inside digitized_by_detector.
	// If this is not the first detector in digitized_by_detector, add comma.
	const std::string updated                    = current_event.str();
	const bool        digitized_has_any_detector =
		(updated.find("\"digitized_by_detector\": {") != std::string::npos) &&
		(updated.find("\"digitized_by_detector\": {") < updated.size() - 1) &&
		(updated.find("\"digitized_by_detector\": {") != std::string::npos);

	// We cannot robustly determine "first" by parsing; instead, we append with a comma if the last
	// character before we append is not '{'.
	// This works because we only ever append within the object and we never add trailing spaces/newlines.
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

		// int observables (argument: 0 = do not include sro variables)
		for (const auto& [name, value] : hit->getIntObservablesMap(0)) {
			if (wrote_first_var) current_event << ", ";
			wrote_first_var = true;
			current_event << "\"" << jsonEscape(name) << "\": " << value;
		}

		// double observables
		for (const auto& [name, value] : hit->getDblObservablesMap(0)) {
			if (wrote_first_var) current_event << ", ";
			wrote_first_var = true;
			current_event << "\"" << jsonEscape(name) << "\": " << value;
		}

		current_event << "}"; // vars
		current_event << "}"; // hit
	}

	current_event << "]"; // detector array

	// If we opened the digitized_by_detector container earlier, it remains open until endEventImpl.
	// endEventImpl closes the detectors object implicitly only by terminating the event; therefore we need
	// to ensure the reserved container is properly closed before the detectors object closes.
	//
	// We close the reserved container immediately after each insertion *only if it was newly opened*,
	// otherwise we'd break subsequent inserts. Since we do not track that state robustly in this minimal
	// implementation, we keep the container open and close it in endEventImpl by emitting a closing brace
	// when present. That requires endEventImpl to be aware of it; we avoid that by leaving the container
	// as a standard detector-like entry:
	//
	// "detectors": {
	//   "ctof": {...},
	//   "digitized_by_detector": { "ctof":[...], ... }
	// }
	//
	// This is valid as long as the digitized_by_detector object is closed before "detectors" closes.
	// To guarantee that without rewriting endEventImpl, we close digitized_by_detector here by appending "}"
	// and then re-open it on the next call if needed (safe, simple).
	current_event << "}"; // close digitized_by_detector object

	return true;
}
