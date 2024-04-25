#ifndef GSTREAMERROOTFACTORY_H
#define GSTREAMERROOTFACTORY_H 1

// gstreamer
#include "gstreamer.h"
#include "gRootTree.h"

// c++
#include <map>
//using std::ofstream;

// ROOT
#include "TFile.h"

class GstreamerRootFactory : public GStreamer
{
public:
	GstreamerRootFactory() {}

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
	TFile *rootfile;

	// return header tree from map. If not there, initialize it.
	// done at startEvent
	GRootTree* getOrInstantiateHeaderTree(const GEventDataCollectionHeader *gheader);
	GRootTree *getOrInstantiateTrueInfoDataTree(const string detectorName,  const GTrueInfoData*  gdata);
	GRootTree *getOrInstantiateDigitizedDataTree(const string detectorName, const GDigitizedData* gdata);

	// instantiated (and their variable maps) during the first event in startEvent
	map<string, GRootTree*> *gRootTrees;

};

#endif // GSTREAMERTXTFACTORY_H

