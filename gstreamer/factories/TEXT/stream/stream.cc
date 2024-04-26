// gstreamer
#include "../gstreamerTEXTFactory.h"

bool GstreamerTextFactory::startStream([[maybe_unused]] const GFrameDataCollection* frameRunData)
{
	if(ofile == nullptr) return false;

	*ofile  << "Frame {" << endl;

	return true;
}




bool GstreamerTextFactory::endStream([[maybe_unused]]  const GFrameDataCollection* frameRunData)
{
	if(ofile == nullptr) return false;

	*ofile  << "}" << endl;

	return true;
}
