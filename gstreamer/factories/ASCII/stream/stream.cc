// gstreamer
#include "../gstreamerASCIIFactory.h"
#include "gstreamerConventions.h"

bool GstreamerTextFactory::startStream([[maybe_unused]] const GFrameDataCollection* frameRunData, const std::shared_ptr<GLogger>& log) {
	if (ofile == nullptr) { log->error(ERR_CANTOPENOUTPUT, "Error: can't open ", ofile); }

	*ofile << "Frame {" << std::endl;

	return true;
}


bool GstreamerTextFactory::endStream([[maybe_unused]] const GFrameDataCollection* frameRunData, const std::shared_ptr<GLogger>& log) {
	if (ofile == nullptr) { log->error(ERR_CANTOPENOUTPUT, "Error: can't open ", ofile); }

	*ofile << "}" << std::endl;

	return true;
}
