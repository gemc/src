// gstreamer
#include "gstreamerROOTFactory.h"
#include "gstreamerConventions.h"

// Non-Doxygen implementation file: behavior is documented in the header.
bool GstreamerRootFactory::startStreamImpl([[maybe_unused]] const GFrameDataCollection* frameRunData) {
	if (rootfile == nullptr) {
		log->error(ERR_CANTOPENOUTPUT, "GstreamerRootFactory::startStream: file is not initialized");
	}

	return true;
}


bool GstreamerRootFactory::endStreamImpl([[maybe_unused]] const GFrameDataCollection* frameRunData) {
	if (rootfile == nullptr) {
		log->error(ERR_CANTOPENOUTPUT, "GstreamerRootFactory::endStream: file is not initialized");
	}

	return true;
}
