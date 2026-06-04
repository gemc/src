// gstreamer
#include "gstreamerJSONFactory.h"
#include "gstreamerConventions.h"

// c++
#include <sstream>

// Implementation summary:
// Start and finalize one JSON event object using the ordered event publish sequence.

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

	// Ensure the top-level JSON document is initialized for event output.
	ensureFileInitializedForType("event");

	// Reset all per-event assembly state.
	is_building_event = true;
	current_event.str(std::string());
	current_event.clear();
	current_event_has_header       = false;
	current_event_has_any_detector = false;
	current_event_has_generated    = false;

	// Cache the event number early so it is available to the root event object.
	event_number = event_data->getHeader()->getG4LocalEvn();

	current_event << "{";
	current_event << "\"event_number\": " << event_number;

	// Open the header object now. publishEventHeaderImpl() fills its contents.
	current_event << ", \"header\": {";
	return true;
}

bool GstreamerJsonFactory::endEventImpl(const std::shared_ptr<GEventDataCollection>& event_data) {
	if (!is_building_event) {
		log->error(ERR_PUBLISH_ERROR, "endEventImpl called without an active event in GstreamerJsonFactory");
		return false;
	}

	// If the caller skipped the header step, emit a minimal fallback so the JSON remains valid.
	if (!current_event_has_header) {
		current_event << "\"timestamp\": \"\", \"thread_id\": -1";
	}

	// Detector publishers leave the "detectors" object open so digitized data
	// can be appended after true-information banks without rewriting strings.
	if (current_event_has_any_detector) {
		current_event << "}";
	}
	current_event << "}";

	// Keep the event schema predictable even when no detector banks were published.
	if (!current_event_has_any_detector) {
		current_event << ", \"detectors\": {}";
	}

	current_event << "}";

	writeTopLevelEntry(current_event.str());

	is_building_event = false;

	(void)event_data;
	return true;
}

bool GstreamerJsonFactory::publishEventGeneratedParticlesImpl(const std::string& bankName,
                                                              const GGeneratedParticleBank& particles) {
	if (!is_building_event) {
		log->error(ERR_PUBLISH_ERROR,
		           "publishEventGeneratedParticlesImpl called without an active event in GstreamerJsonFactory");
		return false;
	}

	if (!current_event_has_generated) {
		current_event << ", \"generated\": {";
		current_event_has_generated = true;
	}
	else {
		current_event << ", ";
	}

	current_event << "\"" << jsonEscape(bankName) << "\": [";

	bool wrote_first_particle = false;
	for (const auto& particle : particles) {
		if (wrote_first_particle) { current_event << ", "; }
		wrote_first_particle = true;

		current_event << "{"
		              << "\"name\": \"" << jsonEscape(particle.name) << "\""
		              << ", \"pid\": " << particle.pid
		              << ", \"type\": " << particle.type
		              << ", \"multiplicity\": " << particle.multiplicity
		              << ", \"p\": " << particle.p
		              << ", \"theta\": " << particle.theta
		              << ", \"phi\": " << particle.phi
		              << ", \"vx\": " << particle.vx
		              << ", \"vy\": " << particle.vy
		              << ", \"vz\": " << particle.vz
		              << "}";
	}

	current_event << "]";
	if (bankName == "generated_tracked") {
		current_event << "}";
	}

	return true;
}
