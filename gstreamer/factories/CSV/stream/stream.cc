// gstreamer
#include "gstreamerCSVFactory.h"
#include "gstreamerConventions.h"

// Non-Doxygen implementation file: behavior is documented in the header.
bool GstreamerCsvFactory::startStreamImpl([[maybe_unused]] const GFrameDataCollection* frameRunData) {
	return true;
}


bool GstreamerCsvFactory::endStreamImpl([[maybe_unused]] const GFrameDataCollection* frameRunData) {
	return true;
}
