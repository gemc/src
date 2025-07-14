// gstreamer 
#include "gstreamer.h"

// gemc
#include "gutilities.h"


const std::vector<std::string> GStreamer::supported_formats = {"jlabsro", "root", "ascii"};

bool GStreamer::is_valid_format(const std::string& format) {
	std::vector<std::string> supported = GStreamer::supported_formats;
	return std::find(supported.begin(), supported.end(), gutilities::convertToLowercase(format)) != supported.end();
}

// pragma todo: pass someting like map<string, bitset> to each detector to decide which data to publish
void GStreamer::publishEventData(const GEventDataCollection* event_data) {

	eventBuffer.push_back(event_data);
	if (eventBuffer.size() >= bufferFlushLimit) {
		flushEventBuffer(); // or call virtual flushImpl() if you want customization
	}

}


// stream an individual frame
void GStreamer::publishFrameRunData(const std::shared_ptr<GLogger>& log, const GFrameDataCollection* frameRunData) {
	// TODO: add more infor like frame number or number of entries in paylod

	log->info(2, "GStreamer::publishFrameRunData:  ",
		gutilities::success_or_fail(startStream(frameRunData)));
	log->info(2, "GStreamer::publishFrameHeader:  ",
		gutilities::success_or_fail(publishFrameHeader(frameRunData->getHeader())));
	log->info(2, "GStreamer::publishPayload:  ",
		gutilities::success_or_fail(publishPayload(frameRunData->getIntegralPayload())));
	log->info(2, "GStreamer::endStream:  ",
		gutilities::success_or_fail(endStream(frameRunData)));

}
