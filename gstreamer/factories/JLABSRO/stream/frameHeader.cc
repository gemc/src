// gstreamer
#include "../gstreamerJLABSROFactory.h"
#include "gstreamerConventions.h"

// c++
#include<vector>

bool GstreamerJSROFactory::publishFrameHeader([[maybe_unused]] const GFrameDataCollectionHeader *gframeHeader, const std::shared_ptr<GLogger>& log)
{
	if (ofile == nullptr) { log->error(ERR_CANTOPENOUTPUT, "Error: can't open ", ofile); }
	
	// fill dataFrameHeader here or in startStream
	//vector<unsigned int> frame_data;
	//	dataFrameHeader = *reinterpret_cast<DataFrameHeader*>(frame_data.data());

	ofile->write(reinterpret_cast<const char*>(frame_data.data()),  sizeof(DataFrameHeader));
	
	return true;
	
	
}
