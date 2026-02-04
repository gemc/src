// gstreamer
#include "gstreamerCSVFactory.h"

// using \n instead of endl so flushing isn't forced at each line
// Non-Doxygen implementation file: behavior is documented in the header.
bool GstreamerCsvFactory::publishEventHeaderImpl([[maybe_unused]] const std::unique_ptr<GEventHeader>& gevent_header) {
	// Saving header vars so we can flatten them in the publish routines.
	timestamp = gevent_header->getTimeStamp();
	thread_id = gevent_header->getThreadID();

	return true;
}
