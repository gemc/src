// gstreamer
#include "gstreamerROOTFactory.h"
#include "gstreamerConventions.h"

// Non-Doxygen implementation file: behavior is documented in the header.
bool GstreamerRootFactory::publishFrameHeaderImpl([[maybe_unused]] const GFrameHeader* gframeHeader) {
	if (rootfile == nullptr) { log->error(ERR_CANTOPENOUTPUT, "GstreamerRootFactory: file is not initialized"); }

	return true;
}
