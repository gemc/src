// gstreamer
#include "gstreamerASCIIFactory.h"
#include "gstreamerConventions.h"

// Implementation summary:
// Write frame-level opening and closing delimiters for the ASCII backend.

bool GstreamerTextFactory::startStreamImpl([[maybe_unused]] const GFrameDataCollection* frameRunData) {
	if (!ofile.is_open()) { log->error(ERR_CANTOPENOUTPUT, "Error: can't open ", filename()); }

	// Start one human-readable frame block.
	ofile << "Frame {" << std::endl;

	return true;
}


bool GstreamerTextFactory::endStreamImpl([[maybe_unused]] const GFrameDataCollection* frameRunData) {
	if (!ofile) { log->error(ERR_CANTOPENOUTPUT, "Error: can't open ", filename()); }

	// Close the frame block opened in startStreamImpl().
	ofile << "}" << std::endl;

	return true;
}