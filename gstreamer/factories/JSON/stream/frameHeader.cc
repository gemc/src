// gstreamer
#include "gstreamerJSONFactory.h"
#include "gstreamerConventions.h"

bool GstreamerJsonFactory::publishFrameHeaderImpl(const GFrameHeader* gframeHeader) {
	if (!is_building_frame) {
		log->error(ERR_PUBLISH_ERROR, "publishFrameHeaderImpl called without an active frame in GstreamerJsonFactory");
		return false;
	}
	if (!gframeHeader) {
		log->error(ERR_PUBLISH_ERROR, "gframeHeader is null in GstreamerJsonFactory::publishFrameHeaderImpl");
		return false;
	}

	// Minimal header encoding (extend as needed).
	current_frame << "\"header\": {"
				  << "\"frame_id\": " << gframeHeader->getFrameID()
				  << "}";

	current_frame_has_header = true;
	return true;
}
