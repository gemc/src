// gstreamer
#include "../gstreamerROOTFactory.h"
#include "gstreamerConventions.h"

bool GstreamerRootFactory::startStream([[maybe_unused]] const GFrameDataCollection *frameRunData, const std::shared_ptr<GLogger>& log) {
	if (rootfile == nullptr) { log->error(ERR_CANTOPENOUTPUT, "GstreamerRootFactory::startStream: file is not initialized"); }

    return true;
}


bool GstreamerRootFactory::endStream([[maybe_unused]] const GFrameDataCollection *frameRunData, const std::shared_ptr<GLogger>& log) {
	if (rootfile == nullptr) { log->error(ERR_CANTOPENOUTPUT, "GstreamerRootFactory::endStream: file is not initialized"); }

    return true;
}
