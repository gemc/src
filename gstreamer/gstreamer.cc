// gstreamer 
#include "gstreamer.h"


// pragma todo: pass someting like map<string, bitset> to each detector to decide which data to publish
map<string, bool> GStreamer::publishEventRunData([[maybe_unused]]  const GOptions *gopts, const vector<GEventDataCollection*> runData) {

	map<string, bool> gstreamReport;

	// TODO: this needs to run if it's requested by the gopts
	// option should be added to write this / or stream this
	// looping over events
	for(auto eventDataCollection: runData) {

		gstreamReport["Event Stream report #1 <startEvent>: "] = startEvent(eventDataCollection);
		gstreamReport["Event Stream report #2 <header>: "]     = publishEventHeader(eventDataCollection->getHeader());

		for (auto& [detectorName, gDataCollection] : *eventDataCollection->getDataCollectionMap() ) {
			// publish true info
			string reportName = "Event Stream report #3 <" + detectorName + "__TrueInfo>: ";
			gstreamReport[reportName] = publishEventTrueInfoData(detectorName, gDataCollection->getTrueInfoData());

			// publish digitized data
			reportName = "Event Stream report #4: <" +  detectorName + "__Digitized>: ";
			gstreamReport[reportName] = publishEventDigitizedData(detectorName, gDataCollection->getDigitizedData());
		}
		gstreamReport["Event Stream report #5 <endEvent>: "] = endEvent(eventDataCollection);
	}

	return gstreamReport;
}


// stream an individual frame
map<string, bool> GStreamer::publishFrameRunData([[maybe_unused]] const GOptions *gopts, const GFrameDataCollection* frameRunData) {
	map<string, bool> gstreamReport;

	gstreamReport["Frame Stream report #1 <startStream>: "] = startStream(frameRunData);
	gstreamReport["Frame Stream report #2 <frameHeader>: "] = publishFrameHeader(frameRunData->getHeader());
	gstreamReport["Frame Stream report #3 <payload>: "]     = publishPayload(frameRunData->getIntegralPayload());
	gstreamReport["Frame Stream report #4 <endStream>: "]   = endStream(frameRunData);

	return gstreamReport;
}
