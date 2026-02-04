// gstreamer
#include "gstreamerJSONFactory.h"
#include "gstreamerConventions.h"

// c++
#include <sstream>

bool GstreamerJsonFactory::startEventImpl(const std::shared_ptr<GEventDataCollection>& event_data) {
	if (!ofile.is_open()) { log->error(ERR_CANTOPENOUTPUT, SFUNCTION_NAME, "Error: can't access ", filename()); }
	if (!event_data) {
		log->error(ERR_PUBLISH_ERROR, "event_data is null in GstreamerJsonFactory::startEventImpl");
		return false;
	}
	if (!event_data->getHeader()) {
		log->error(ERR_PUBLISH_ERROR, "event header is null in GstreamerJsonFactory::startEventImpl");
		return false;
	}

	ensureFileInitializedForType("event");

	// Reset per-event assembly.
	is_building_event = true;
	current_event.str(std::string());
	current_event.clear();
	current_event_has_header       = false;
	current_event_has_any_detector = false;

	// Cache a few commonly used fields for convenience / diagnostics.
	event_number = event_data->getHeader()->getG4LocalEvn();

	// Build the event object incrementally.
	current_event << "{";
	current_event << "\"event_number\": " << event_number;

	// Header and detectors will be appended by subsequent publish calls.
	current_event << ", \"header\": {";
	// Header fields will be inserted by publishEventHeaderImpl; keep the object open for now.
	return true;
}

bool GstreamerJsonFactory::endEventImpl(const std::shared_ptr<GEventDataCollection>& event_data) {
	if (!is_building_event) {
		log->error(ERR_PUBLISH_ERROR, "endEventImpl called without an active event in GstreamerJsonFactory");
		return false;
	}

	// Close header object if it was never completed (should not happen in normal flow).
	if (!current_event_has_header) {
		// Emit a minimal header block so the JSON remains valid.
		current_event << "\"timestamp\": \"\", \"thread_id\": -1";
	}
	current_event << "}"; // close "header"

	// Ensure detectors object exists even if empty (predictable schema).
	if (!current_event_has_any_detector) {
		current_event << ", \"detectors\": {}";
	}

	current_event << "}"; // close event object

	// Write to file.
	writeTopLevelEntry(current_event.str());

	// Reset state.
	is_building_event = false;

	// event_data is intentionally unused (kept for signature consistency and future diagnostics).
	(void)event_data;

	return true;
}
