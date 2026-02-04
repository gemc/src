// gstreamer
#include "gstreamerJLABSROFactory.h"
#include "gstreamerConventions.h"

// c++
#include<vector>

// Non-Doxygen implementation file: behavior is documented in the header.
bool GstreamerJSROFactory::publishFrameHeaderImpl([[maybe_unused]] const GFrameHeader* gframeHeader) {
	if (ofile == nullptr) { log->error(ERR_CANTOPENOUTPUT, "Error: can't open ", ofile); }

	// fill dataFrameHeader here or in startStream
	//vector<unsigned int> frame_data;
	//	dataFrameHeader = *reinterpret_cast<DataFrameHeader*>(frame_data.data());

	ofile->write(reinterpret_cast<const char*>(frame_data.data()), sizeof(DataFrameHeader));

	return true;
}
