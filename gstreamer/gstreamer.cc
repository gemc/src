// gstreamer 
#include "gstreamer.h"

// glibrary
#include "gutilities.h"

const vector<string> GStreamer::supported_formats = {"jlabsro", "root", "text"};

bool GStreamer::is_valid_format(const string& format) {
    vector < string > supported = GStreamer::supported_formats;
    return std::find(supported.begin(), supported.end(), gutilities::convertToLowercase(format)) != supported.end();
}

// pragma todo: pass someting like map<string, bitset> to each detector to decide which data to publish
map<string, bool> GStreamer::publishEventRunData(std::shared_ptr<GLogger>& log, const vector<GEventDataCollection *>& runData) {

    map<string, bool> gstreamReport;

	// looping over events
    for (auto eventDataCollection: runData) {

        gstreamReport["Event Stream report #1 <startEvent>: "] = startEvent(eventDataCollection, log);
        gstreamReport["Event Stream report #2 <header>: "] = publishEventHeader(eventDataCollection->getHeader(), log);

        for (auto &[detectorName, gDataCollection]: *eventDataCollection->getDataCollectionMap()) {
            // publish true info
            string reportName = "Event Stream report #3 <" + detectorName + "__TrueInfo>: ";
            gstreamReport[reportName] = publishEventTrueInfoData(detectorName, gDataCollection->getTrueInfoData(), log);

            // publish digitized data
            reportName = "Event Stream report #4: <" + detectorName + "__Digitized>: ";
            gstreamReport[reportName] = publishEventDigitizedData(detectorName, gDataCollection->getDigitizedData(), log);
        }
        gstreamReport["Event Stream report #5 <endEvent>: "] = endEvent(eventDataCollection, log);
    }

    return gstreamReport;
}


// stream an individual frame
map<string, bool> GStreamer::publishFrameRunData(const std::shared_ptr<GLogger>& log, const GFrameDataCollection *frameRunData) {
    map<string, bool> gstreamReport;

    gstreamReport["Frame Stream report #1 <startStream>: "] = startStream(frameRunData, log);
    gstreamReport["Frame Stream report #2 <frameHeader>: "] = publishFrameHeader(frameRunData->getHeader(), log);
    gstreamReport["Frame Stream report #3 <payload>: "] = publishPayload(frameRunData->getIntegralPayload(), log);
    gstreamReport["Frame Stream report #4 <endStream>: "] = endStream(frameRunData, log);

    return gstreamReport;
}
