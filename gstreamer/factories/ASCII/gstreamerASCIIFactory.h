#ifndef GSTREAMERTXTFACTORY_H
#define GSTREAMERTXTFACTORY_H 1

// gstreamer
#include "gstreamer.h"

#include <fstream>
using std::ofstream;

class GstreamerTextFactory : public GStreamer
{
public:
	GstreamerTextFactory() {}

private:
	// open and close the output media
	bool openConnection();
	bool closeConnection();

	// event streams
	// start and end each event
	bool startEvent(const GEventDataCollection* eventData);
	bool endEvent(const GEventDataCollection* eventData);

	// write the header
	bool publishEventHeader(const GEventDataCollectionHeader *gheader);

	// vector index is hit number
	bool publishEventTrueInfoData( const string detectorName, const vector<GTrueInfoData*>*  trueInfoData);
	bool publishEventDigitizedData(const string detectorName, const vector<GDigitizedData*>* digitizedData);

	// frame streams
	bool startStream(const GFrameDataCollection* frameRunData);
	bool endStream(const GFrameDataCollection* frameRunData);
	bool publishFrameHeader(const GFrameDataCollectionHeader *gframeHeader);
	bool publishPayload(const vector<GIntegralPayload*> *payload);


private:
	ofstream *ofile = nullptr;
};

#endif // GSTREAMERTXTFACTORY_H

