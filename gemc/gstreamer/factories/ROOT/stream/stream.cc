// gstreamer
#include "gstreamerROOTFactory.h"
#include "gstreamerConventions.h"

// Implementation summary:
// Frame-stream hooks are currently placeholders for the ROOT backend.

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