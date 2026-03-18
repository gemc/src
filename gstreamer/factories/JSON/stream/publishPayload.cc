// gstreamer
#include "gstreamerJSONFactory.h"
#include "gstreamerConventions.h"

// Implementation summary:
// Append the frame payload block to the JSON object currently being assembled.

bool GstreamerJsonFactory::publishPayloadImpl(const std::vector<GIntegralPayload*>* payload) {
	if (!is_building_frame) {
		log->error(ERR_PUBLISH_ERROR, "publishPayloadImpl called without an active frame in GstreamerJsonFactory");
		return false;
	}
	if (!payload) {
		log->error(ERR_PUBLISH_ERROR, "payload is null in GstreamerJsonFactory::publishPayloadImpl");
		return false;
	}

	// Separate the payload block from the header block if both are present.
	if (current_frame_has_header) current_frame << ", ";

	current_frame << "\"payload\": [";

	bool wrote_first = false;
	for (const auto* pl : *payload) {
		if (!pl) continue;

		if (wrote_first) current_frame << ", ";
		wrote_first = true;

		// Each payload object is written as one JSON array of integers.
		current_frame << "[";
		const auto vec = pl->getPayload();

		for (size_t i = 0; i < vec.size(); i++) {
			current_frame << vec[i];
			if (i + 1 < vec.size()) current_frame << ", ";
		}
		current_frame << "]";
	}

	current_frame << "]";

	current_frame_has_payload = true;
	return true;
}