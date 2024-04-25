// gstreamer
#include "../gstreamerTEXTFactory.h"

bool GstreamerTextFactory::publishFrameHeader(const GFrameDataCollectionHeader *gframeHeader) 
{
	if(ofile == nullptr) return false;

	*ofile << GTAB << "Frame Header  {" << endl;
	*ofile << GTABTAB << " frameID: " << gframeHeader->getFrameID() << endl;
	*ofile << GTAB << "}" << endl;

	return true;
}
