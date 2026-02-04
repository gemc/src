// gstreamer
#include "gstreamerCSVFactory.h"
#include "gstreamerConventions.h"

// using \n instead of endl so flushing isn't forced at each line
// Non-Doxygen implementation file: behavior is documented in the header.
bool GstreamerCsvFactory::publishFrameHeaderImpl([[maybe_unused]] const GFrameHeader* gframeHeader) {
	return true;
}
