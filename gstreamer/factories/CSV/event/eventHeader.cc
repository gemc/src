// gstreamer
#include "gstreamerCSVFactory.h"

// using \n instead of endl so flushing isn't forced at each line
bool GstreamerCsvFactory::publishEventHeaderImpl([[maybe_unused]] const std::unique_ptr<GEventHeader>& gheader) {

	// saving header vars so we can flatten them in the publish routines
	timestamp = gheader->getTimeStamp();
	thread_id = gheader->getThreadID();

	return true;
}
