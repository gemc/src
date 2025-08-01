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
void GStreamer::publishEventData(const std::shared_ptr<GEventDataCollection>& event_data) {
	// event_data and its header must not be null
	if (!event_data) { log->error(ERR_PUBLISH_ERROR, "event data is null in GStreamer::publishEventData"); }
	if (!event_data->getHeader()) { log->error(ERR_PUBLISH_ERROR, "event header is null in GStreamer::publishEventData"); }

	// add to the buffer
	eventBuffer.emplace_back(event_data);

	// flush if the buffer is full
	if (eventBuffer.size() >= bufferFlushLimit) { flushEventBuffer(); }
}

void GStreamer::flushEventBuffer() {
	log->info(2, "GStreamer::flushEventBuffer -> flushing ", eventBuffer.size(), " events to file");

	// events are read only by the streamer
	for (const auto& eventData : eventBuffer) {

		log->info(2, "GStreamer::publishEventData->startEvent: ",
		          gutilities::success_or_fail(startEvent(eventData)));

		log->info(2, "GStreamer::publishEventData->publishEventHeader -> ",
		          gutilities::success_or_fail(publishEventHeader(eventData->getHeader())));


		for (const auto& [detectorName, gDataCollection] : eventData->getDataCollectionMap()) {
			const GDataCollection* tdptr = gDataCollection.get();

			// extract the vector of raw pointers to publish
			std::vector<const GTrueInfoData*>  trueInfoPtrs;
			std::vector<const GDigitizedData*> digitizedPtrs;
			trueInfoPtrs.reserve(tdptr->getTrueInfoData().size());
			digitizedPtrs.reserve(tdptr->getDigitizedData().size());

			for (const auto& hit : tdptr->getTrueInfoData()) { trueInfoPtrs.push_back(hit.get()); }
			for (const auto& hit : tdptr->getDigitizedData()) { digitizedPtrs.push_back(hit.get()); }


			log->info(2, "GStreamer::publishEventData->publishEventTrueInfoData for detector -> ", detectorName,
			          gutilities::success_or_fail(publishEventTrueInfoData(detectorName, trueInfoPtrs)));

			log->info(2, "GStreamer::publishEventData->publishEventDigitizedData for detector -> ", detectorName,
			          gutilities::success_or_fail(publishEventDigitizedData(detectorName, digitizedPtrs)));
		}

		log->info(2, "GStreamer::endEvent -> ", gutilities::success_or_fail(endEvent(eventData)));
	}

	eventBuffer.clear();
}

// stream an individual frame
void GStreamer::publishFrameRunData(const std::shared_ptr<GFrameDataCollection>& frameRunData) {
	// TODO: add more infor like frame number or number of entries in paylod

	// log->info(2, "GStreamer::publishFrameRunData:  ",
	// 	gutilities::success_or_fail(startStream(frameRunData)));
	// log->info(2, "GStreamer::publishFrameHeader:  ",
	// 	gutilities::success_or_fail(publishFrameHeader(frameRunData->getHeader())));
	// log->info(2, "GStreamer::publishPayload:  ",
	// 	gutilities::success_or_fail(publishPayload(frameRunData->getIntegralPayload())));
	// log->info(2, "GStreamer::endStream:  ",
	// 	gutilities::success_or_fail(endStream(frameRunData)));
}
