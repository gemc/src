// gstreamer
#include "gstreamer.h"

// gemc
#include "gutilities.h"

// Implementation summary:
// Common base-class logic for format validation, buffered event publication,
// and immediate run publication. Concrete serialization remains in plugin hooks.

const std::vector<std::string>& GStreamer::supported_formats() {
	// Keep this list aligned with the available gstreamer_<format>_plugin factories.
	static const std::vector<std::string> formats = {"jlabsro", "root", "ascii", "csv", "json"};
	return formats;
}

bool GStreamer::is_valid_format(const std::string& format) {
	const auto& supported = GStreamer::supported_formats();
	const auto  f         = gutilities::convertToLowercase(format);
	return std::find(supported.begin(), supported.end(), f) != supported.end();
}


// pragma todo: pass someting like map<string, bitset> to each detector to decide which data to publish
void GStreamer::publishEventData(const std::shared_ptr<GEventDataCollection>& event_data) {
	// The event collection and its header are required for any plugin to publish
	// a meaningful event record.
	if (!event_data) { log->error(ERR_PUBLISH_ERROR, "event data is null in GStreamer::publishEventData"); }
	if (!event_data->getHeader()) {
		log->error(ERR_PUBLISH_ERROR, "event header is null in GStreamer::publishEventData");
	}

	// Retain ownership of the event until the buffer is flushed. This guarantees
	// that raw pointers extracted later from hit collections remain valid.
	eventBuffer.emplace_back(event_data);

	// Once the configured threshold is reached, publish all buffered events in one pass.
	if (eventBuffer.size() >= bufferFlushLimit) { flushEventBuffer(); }
}


// Implementation summary:
// Run data are published immediately instead of being buffered. The publish order
// mirrors the base-class run sequence: start, header, detector banks, end.
void GStreamer::publishRunData(const std::shared_ptr<GRunDataCollection>& run_data) {
	log->info(2, "GStreamer::publishRunData->startRun: ",
			  gutilities::success_or_fail(startRun(run_data)));

	log->info(2, SFUNCTION_NAME, "->publishRunHeader -> ",
		  gutilities::success_or_fail(publishRunHeader(run_data->getHeader())));

	// Iterate over each detector collection and expose it to the plugin as a
	// vector of raw pointers. The owning run collection remains alive throughout
	// this method call.
	for (const auto& [sdname, gDataCollection] : run_data->getDataCollectionMap()) {
		const GDataCollection* tdptr = gDataCollection.get();

		// Extract digitized hits into a flat raw-pointer view expected by the hooks.
		std::vector<const GDigitizedData*> digitizedPtrs;
		digitizedPtrs.reserve(tdptr->getDigitizedData().size());

		for (const auto& hit : tdptr->getDigitizedData()) { digitizedPtrs.push_back(hit.get()); }

		log->info(2, SFUNCTION_NAME, "->publishEventDigitizedData for detector -> ", sdname,
				  gutilities::success_or_fail(publishRunDigitizedData(sdname, digitizedPtrs)));
	}

	log->info(2, "GStreamer::endEvent -> ",
			  gutilities::success_or_fail(endRun(run_data)));
}


void GStreamer::flushEventBuffer() {
	log->info(2, "GStreamer::flushEventBuffer -> flushing ", eventBuffer.size(), " events to file");

	// Each buffered event is treated as read-only while the plugin hooks serialize it.
	// The buffer's shared_ptr ownership keeps all event-owned hit objects alive during the flush.
	for (const auto& eventData : eventBuffer) {
		log->info(2, SFUNCTION_NAME, "->startEvent: ",
				  gutilities::success_or_fail(startEvent(eventData)));

		log->info(2, SFUNCTION_NAME, "->publishEventHeader -> ",
				  gutilities::success_or_fail(publishEventHeader(eventData->getHeader())));

		// Publish one detector collection at a time.
		for (const auto& [sdname, gDataCollection] : eventData->getDataCollectionMap()) {
			const GDataCollection* tdptr = gDataCollection.get();

			// Convert ownership-bearing containers into temporary raw-pointer views.
			// Plugins consume these views immediately and do not own the pointed data.
			std::vector<const GTrueInfoData*>  trueInfoPtrs;
			std::vector<const GDigitizedData*> digitizedPtrs;
			trueInfoPtrs.reserve(tdptr->getTrueInfoData().size());
			digitizedPtrs.reserve(tdptr->getDigitizedData().size());

			for (const auto& hit : tdptr->getTrueInfoData()) { trueInfoPtrs.push_back(hit.get()); }
			for (const auto& hit : tdptr->getDigitizedData()) { digitizedPtrs.push_back(hit.get()); }

			log->info(2, SFUNCTION_NAME, "->publishEventTrueInfoData for detector -> ", sdname,
					  gutilities::success_or_fail(publishEventTrueInfoData(sdname, trueInfoPtrs)));

			log->info(2, SFUNCTION_NAME, "->publishEventDigitizedData for detector -> ", sdname,
					  gutilities::success_or_fail(publishEventDigitizedData(sdname, digitizedPtrs)));
		}

		log->info(2, "GStreamer::endEvent -> ", gutilities::success_or_fail(endEvent(eventData)));
	}

	// All buffered events have now been handed to the plugin hooks.
	eventBuffer.clear();
}

// stream an individual frame
// void GStreamer::publishFrameRunData(const std::shared_ptr<GFrameDataCollection>& frameRunData) {
// TODO: add more infor like frame number or number of entries in paylod

// log->info(2, "GStreamer::publishFrameRunData:  ",
// 	gutilities::success_or_fail(startStream(frameRunData)));
// log->info(2, "GStreamer::publishFrameHeader:  ",
// 	gutilities::success_or_fail(publishFrameHeader(frameRunData->getHeader())));
// log->info(2, "GStreamer::publishPayload:  ",
// 	gutilities::success_or_fail(publishPayload(frameRunData->getIntegralPayload())));
// log->info(2, "GStreamer::endStream:  ",
// 	gutilities::success_or_fail(endStream(frameRunData)));
// }