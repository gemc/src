// gstreamer
#include "gstreamerCSVFactory.h"
#include "gstreamerConventions.h"

// Implementation summary:
// Frame streaming is currently a no-op for the CSV backend.

bool GstreamerCsvFactory::startStreamImpl([[maybe_unused]] const GFrameDataCollection* frameRunData) {
	return true;
}


bool GstreamerCsvFactory::endStreamImpl([[maybe_unused]] const GFrameDataCollection* frameRunData) {
	return true;
}