// gstreamer
#include "gstreamerROOTFactory.h"
#include "gstreamerConventions.h"

// Implementation summary:
// Frame-header publication is currently a placeholder for the ROOT backend.

bool GstreamerRootFactory::publishFrameHeaderImpl([[maybe_unused]] const GFrameHeader* gframeHeader) {
	if (rootfile == nullptr) { log->error(ERR_CANTOPENOUTPUT, "GstreamerRootFactory: file is not initialized"); }

	return true;
}