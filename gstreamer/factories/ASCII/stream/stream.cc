// gstreamer
#include "../gstreamerASCIIFactory.h"
#include "gstreamerConventions.h"

bool GstreamerTextFactory::startStreamImpl([[maybe_unused]] const GFrameDataCollection* frameRunData) {
	if (ofile == nullptr) { log->error(ERR_CANTOPENOUTPUT, "Error: can't open ", ofile); }

	*ofile << "Frame {" << std::endl;

	return true;
}


bool GstreamerTextFactory::endStreamImpl([[maybe_unused]] const GFrameDataCollection* frameRunData) {
	if (ofile == nullptr) { log->error(ERR_CANTOPENOUTPUT, "Error: can't open ", ofile); }

	*ofile << "}" << std::endl;

	return true;
}
