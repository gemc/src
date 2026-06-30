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
	current_event_digitized_entries.clear();

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

	// If the caller skipped the header step, emit a minimal fallback and close the header
	// object (startEventImpl opened it; publishEventHeaderImpl normally writes its fields
	// and closes it).
	if (!current_event_has_header) {
		current_event << "\"timestamp\": \"\", \"thread_id\": -1}";
	}

	// Ensure a "detectors" object exists so the schema stays predictable and any buffered
	// digitized data has a place to live, even when no true-information banks were published.
	bool detectors_has_content = current_event_has_any_detector;
	if (!current_event_has_any_detector) {
		current_event << ", \"detectors\": {";
		current_event_has_any_detector = true;
	}

	// Emit all buffered digitized detector arrays as a single, well-formed
	// "digitized_by_detector" object nested inside "detectors".
	if (!current_event_digitized_entries.empty()) {
		if (detectors_has_content) { current_event << ", "; }
		current_event << "\"digitized_by_detector\": {";
		bool first = true;
		for (const auto& entry : current_event_digitized_entries) {
			if (!first) { current_event << ", "; }
			first = false;
			current_event << entry;
		}
		current_event << "}";
	}

	current_event << "}"; // close "detectors"
	current_event << "}"; // close the event object

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

bool GstreamerJsonFactory::publishEventAncestorsImpl(const GAncestorBank& ancestors) {
	if (!is_building_event) {
		log->error(ERR_PUBLISH_ERROR,
		           "publishEventAncestorsImpl called without an active event in GstreamerJsonFactory");
		return false;
	}

	current_event << ", \"ancestors\": [";
	bool first = true;
	for (const auto& ancestor : ancestors) {
		if (!first) { current_event << ", "; }
		first = false;
		current_event << "{\"pid\": " << ancestor.pid
		              << ", \"tid\": " << ancestor.tid
		              << ", \"mtid\": " << ancestor.mtid
		              << ", \"trackE\": " << ancestor.trackE
		              << ", \"px\": " << ancestor.px
		              << ", \"py\": " << ancestor.py
		              << ", \"pz\": " << ancestor.pz
		              << ", \"vx\": " << ancestor.vx
		              << ", \"vy\": " << ancestor.vy
		              << ", \"vz\": " << ancestor.vz
		              << "}";
	}
	current_event << "]";
	return true;
}
