// gstreamer
#include "../gstreamerASCIIFactory.h"
#include "gstreamerConventions.h"

bool GstreamerTextFactory::publishFrameHeader(const GFrameDataCollectionHeader *gframeHeader, const std::shared_ptr<GLogger>& log)
{
	if (ofile == nullptr) { log->error(ERR_CANTOPENOUTPUT, "Error: can't open ", ofile); }

	*ofile << GTAB << "Frame Header  {" << std::endl;
	*ofile << GTABTAB << " frameID: " << gframeHeader->getFrameID() << std::endl;
	*ofile << GTAB << "}" << std::endl;

	return true;
}
