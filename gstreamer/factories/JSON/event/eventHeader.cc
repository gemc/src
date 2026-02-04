// gstreamer
#include "gstreamerJSONFactory.h"
#include "gstreamerConventions.h"

bool GstreamerJsonFactory::publishEventHeaderImpl(const std::unique_ptr<GEventHeader>& gevent_header) {
	if (!is_building_event) {
		log->error(ERR_PUBLISH_ERROR, "publishEventHeaderImpl called without an active event in GstreamerJsonFactory");
		return false;
	}
	if (!gevent_header) {
		log->error(ERR_PUBLISH_ERROR, "gevent_header is null in GstreamerJsonFactory::publishEventHeaderImpl");
		return false;
	}

	// Store header fields for other routines (CSV factory does something similar).
	timestamp = gevent_header->getTimeStamp();
	thread_id = gevent_header->getThreadID();

	// Close/open header object content consistently.
	// Note: startEventImpl already emitted:  , "header": {
	current_event << "\"timestamp\": \"" << jsonEscape(timestamp) << "\""
				  << ", \"thread_id\": " << thread_id
				  << ", \"g4local_event\": " << gevent_header->getG4LocalEvn();

	current_event_has_header = true;
	return true;
}
