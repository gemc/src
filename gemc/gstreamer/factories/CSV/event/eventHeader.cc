// gstreamer
#include "gstreamerCSVFactory.h"

// Implementation summary:
// Cache event-header values so later publish steps can flatten them into CSV rows.

bool GstreamerCsvFactory::publishEventHeaderImpl([[maybe_unused]] const std::unique_ptr<GEventHeader>& gevent_header) {
	timestamp = gevent_header->getTimeStamp();
	thread_id = gevent_header->getThreadID();

	return true;
}