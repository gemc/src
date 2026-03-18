// gstreamer
#include "gstreamerASCIIFactory.h"
#include "gstreamerConventions.h"

// Implementation summary:
// Serialize the frame header as an indented text block.
// Use '\n' instead of std::endl so each line does not force a flush.

bool GstreamerTextFactory::publishFrameHeaderImpl(const GFrameHeader* gframeHeader) {
	if (!ofile.is_open()) { log->error(ERR_CANTOPENOUTPUT, SFUNCTION_NAME, "Error: can't access ", filename()); }

	ofile << GTAB << "Frame Header  {\n";
	ofile << GTABTAB << " frameID: " << gframeHeader->getFrameID() << "\n";
	ofile << GTAB << "}\n";

	return true;
}