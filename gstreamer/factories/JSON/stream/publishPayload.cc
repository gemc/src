// gstreamer
#include "gstreamerJSONFactory.h"
#include "gstreamerConventions.h"

bool GstreamerJsonFactory::publishPayloadImpl(const std::vector<GIntegralPayload*>* payload) {
	if (!is_building_frame) {
		log->error(ERR_PUBLISH_ERROR, "publishPayloadImpl called without an active frame in GstreamerJsonFactory");
		return false;
	}
	if (!payload) {
		log->error(ERR_PUBLISH_ERROR, "payload is null in GstreamerJsonFactory::publishPayloadImpl");
		return false;
	}

	// Separate header from payload if header was written.
	if (current_frame_has_header) current_frame << ", ";

	current_frame << "\"payload\": [";

	bool wrote_first = false;
	for (const auto* pl : *payload) {
		if (!pl) continue;

		if (wrote_first) current_frame << ", ";
		wrote_first = true;

		// Payload is a vector<int>. Encode as JSON array.
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
