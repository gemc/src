// gstreamer
#include "gstreamerASCIIFactory.h"
#include "gstreamerConventions.h"

// using \n instead of endl so flushing isn't forced at each line
// Non-Doxygen implementation file: behavior is documented in the header.
bool GstreamerTextFactory::publishFrameHeaderImpl(const GFrameHeader* gframeHeader) {
	if (!ofile.is_open()) { log->error(ERR_CANTOPENOUTPUT, SFUNCTION_NAME, "Error: can't access ", filename()); }

	ofile << GTAB << "Frame Header  {\n";
	ofile << GTABTAB << " frameID: " << gframeHeader->getFrameID() << "\n";
	ofile << GTAB << "}\n";

	return true;
}
