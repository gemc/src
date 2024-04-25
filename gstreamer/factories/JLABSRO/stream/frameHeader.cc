// gstreamer
#include "../gstreamerJLABSROFactory.h"

// c++
#include<vector>
#include <iostream>

bool GstreamerJSROFactory::publishFrameHeader([[maybe_unused]] const GFrameDataCollectionHeader *gframeHeader)
{
	if(ofile == nullptr) return false;
	
	// fill dataFrameHeader here or in startStream
	//vector<unsigned int> frame_data;
	//	dataFrameHeader = *reinterpret_cast<DataFrameHeader*>(frame_data.data());

	ofile->write(reinterpret_cast<const char*>(frame_data.data()),  sizeof(DataFrameHeader));
	
	return true;
	
	
}
