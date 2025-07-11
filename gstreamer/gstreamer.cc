// gstreamer 
#include "gstreamer.h"

// glibrary
#include "gutilities.h"

using std::string;
using std::vector;
using std::map;

const vector<string> GStreamer::supported_formats = {"jlabsro", "root", "ascii"};

bool GStreamer::is_valid_format(const string& format) {
    vector < string > supported = GStreamer::supported_formats;
    return std::find(supported.begin(), supported.end(), gutilities::convertToLowercase(format)) != supported.end();
}

// pragma todo: pass someting like map<string, bitset> to each detector to decide which data to publish
map<string, bool> GStreamer::publishEventRunData(const vector<GEventDataCollection *>& runData) {

    map<string, bool> gstreamReport;

	// looping over events
    for (auto eventDataCollection: runData) {

        gstreamReport["Event Stream report #1 <startEvent>: "] = startEvent(eventDataCollection);
        gstreamReport["Event Stream report #2 <header>: "] = publishEventHeader(eventDataCollection->getHeader());

        for (auto &[detectorName, gDataCollection]: *eventDataCollection->getDataCollectionMap()) {
            // publish true info
            string reportName = "Event Stream report #3 <" + detectorName + "__TrueInfo>: ";
            gstreamReport[reportName] = publishEventTrueInfoData(detectorName, gDataCollection->getTrueInfoData());

            // publish digitized data
            reportName = "Event Stream report #4: <" + detectorName + "__Digitized>: ";
            gstreamReport[reportName] = publishEventDigitizedData(detectorName, gDataCollection->getDigitizedData());
        }
        gstreamReport["Event Stream report #5 <endEvent>: "] = endEvent(eventDataCollection);
    }

    return gstreamReport;
}


// stream an individual frame
map<string, bool> GStreamer::publishFrameRunData(const std::shared_ptr<GLogger>& log, const GFrameDataCollection *frameRunData) {
    map<string, bool> gstreamReport;

    gstreamReport["Frame Stream report #1 <startStream>: "] = startStream(frameRunData);
    gstreamReport["Frame Stream report #2 <frameHeader>: "] = publishFrameHeader(frameRunData->getHeader());
    gstreamReport["Frame Stream report #3 <payload>: "] = publishPayload(frameRunData->getIntegralPayload());
    gstreamReport["Frame Stream report #4 <endStream>: "] = endStream(frameRunData);

    return gstreamReport;
}
