// gstreamer
#include "../gstreamerROOTFactory.h"
#include "gstreamerConventions.h"

bool GstreamerRootFactory::publishFrameHeader([[maybe_unused]] const GFrameDataCollectionHeader *gframeHeader, const std::shared_ptr<GLogger>& log) {
	if (rootfile == nullptr) { log->error(ERR_CANTOPENOUTPUT, "GstreamerRootFactory: file is not initialized"); }

    return true;
}
