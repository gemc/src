// gstreamer
#include "gstreamerCSVFactory.h"
#include "gstreamerConventions.h"

// using \n instead of endl so flushing isn't forced at each line
bool GstreamerCsvFactory::publishFrameHeaderImpl(const GFrameHeader* gframeHeader) {


	return true;
}
