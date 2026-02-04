// gstreamer
#include "gstreamerJSONFactory.h"
#include "gstreamerConventions.h"

bool GstreamerJsonFactory::startStreamImpl(const GFrameDataCollection* frameRunData) {
	if (!ofile.is_open()) { log->error(ERR_CANTOPENOUTPUT, SFUNCTION_NAME, "Error: can't access ", filename()); }
	if (!frameRunData) { log->error(ERR_PUBLISH_ERROR, "frameRunData is null in GstreamerJsonFactory::startStreamImpl"); }

	ensureFileInitializedForType("stream");

	is_building_frame     = true;
	current_frame.str(std::string());
	current_frame.clear();
	current_frame_has_header  = false;
	current_frame_has_payload = false;

	current_frame << "{";
	return true;
}

bool GstreamerJsonFactory::endStreamImpl(const GFrameDataCollection* frameRunData) {
	if (!is_building_frame) {
		log->error(ERR_PUBLISH_ERROR, "endStreamImpl called without an active frame in GstreamerJsonFactory");
		return false;
	}

	// Close any missing blocks with empty placeholders.
	if (!current_frame_has_header) {
		if (current_frame.str().size() > 1) current_frame << ", ";
		current_frame << "\"header\": {}";
	}
	if (!current_frame_has_payload) {
		if (current_frame.str().size() > 1) current_frame << ", ";
		current_frame << "\"payload\": []";
	}

	current_frame << "}";

	writeTopLevelEntry(current_frame.str());

	is_building_frame = false;

	(void)frameRunData;
	return true;
}
