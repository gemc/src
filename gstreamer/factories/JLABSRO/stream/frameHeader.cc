// gstreamer
#include "gstreamerJLABSROFactory.h"
#include "gstreamerConventions.h"

// c++
#include<vector>

// Implementation summary:
// Write only the packed binary header section of the current frame record.

bool GstreamerJSROFactory::publishFrameHeaderImpl([[maybe_unused]] const GFrameHeader* gframeHeader) {
	if (ofile == nullptr) { log->error(ERR_CANTOPENOUTPUT, "Error: can't open ", ofile); }

	ofile->write(reinterpret_cast<const char*>(frame_data.data()), sizeof(DataFrameHeader));

	return true;
}