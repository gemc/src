// gstreamer
#include "../gstreamerASCIIFactory.h"
#include "gstreamerConventions.h"

bool GstreamerTextFactory::startStreamImpl([[maybe_unused]] const GFrameDataCollection* frameRunData) {
	if (!ofile.is_open()) { log->error(ERR_CANTOPENOUTPUT, "Error: can't open ", filename()); }

	ofile << "Frame {" << std::endl;

	return true;
}


bool GstreamerTextFactory::endStreamImpl([[maybe_unused]] const GFrameDataCollection* frameRunData) {
	if (!ofile) { log->error(ERR_CANTOPENOUTPUT, "Error: can't open ", filename()); }

	ofile << "}" << std::endl;

	return true;
}
