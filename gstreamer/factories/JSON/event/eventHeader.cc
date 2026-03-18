// gstreamer
#include "gstreamerJSONFactory.h"
#include "gstreamerConventions.h"

// Implementation summary:
// Append the event header fields to the JSON event object currently under construction.

bool GstreamerJsonFactory::publishEventHeaderImpl(const std::unique_ptr<GEventHeader>& gevent_header) {
	if (!is_building_event) {
		log->error(ERR_PUBLISH_ERROR, "publishEventHeaderImpl called without an active event in GstreamerJsonFactory");
		return false;
	}
	if (!gevent_header) {
		log->error(ERR_PUBLISH_ERROR, "gevent_header is null in GstreamerJsonFactory::publishEventHeaderImpl");
		return false;
	}

	// Cache selected values because other publish steps may use them for diagnostics
	// or schema shaping.
	timestamp = gevent_header->getTimeStamp();
	thread_id = gevent_header->getThreadID();

	// startEventImpl() already emitted:
	//   { "event_number": ..., "header": {
	// so this method only appends the header fields themselves.
	current_event << "\"timestamp\": \"" << jsonEscape(timestamp) << "\""
				  << ", \"thread_id\": " << thread_id
				  << ", \"g4local_event\": " << gevent_header->getG4LocalEvn();

	current_event_has_header = true;
	return true;
}