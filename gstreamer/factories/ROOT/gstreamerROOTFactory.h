#pragma once

// gstreamer
#include "gstreamer.h"
#include "gRootTree.h"

// ROOT
#include "TFile.h"

class GstreamerRootFactory : public GStreamer {
public:
	GstreamerRootFactory() = default;

private:
	// open and close the output media
	bool openConnection(const std::shared_ptr<GLogger>& log) override;
	bool closeConnection(const std::shared_ptr<GLogger>& log) override;

	// event streams
	// start and end each event
	bool startEvent(const GEventDataCollection* eventData, const std::shared_ptr<GLogger>& log) override;
	bool endEvent(const GEventDataCollection* eventData, const std::shared_ptr<GLogger>& log) override;

	// write the header
	bool publishEventHeader(const GEventDataCollectionHeader* gheader, std::shared_ptr<GLogger>& log) override;

	// vector index is hit number
	bool publishEventTrueInfoData(const string detectorName, const vector<GTrueInfoData*>* trueInfoData,  std::shared_ptr<GLogger>& log) override;
	bool publishEventDigitizedData(const string detectorName, const vector<GDigitizedData*>* digitizedData,  std::shared_ptr<GLogger>& log) override;

	// frame streams
	bool startStream(const GFrameDataCollection* frameRunData, const std::shared_ptr<GLogger>& log) override;
	bool endStream(const GFrameDataCollection* frameRunData, const std::shared_ptr<GLogger>& log) override;
	bool publishFrameHeader(const GFrameDataCollectionHeader* gframeHeader, const std::shared_ptr<GLogger>& log) override;
	bool publishPayload(const vector<GIntegralPayload*>* payload, const std::shared_ptr<GLogger>& logs) override;

private:
	TFile* rootfile = nullptr; // ROOT file pointer

	// return the header tree from the map. If it's not there, initialize it.
	// executed at startEvent
	GRootTree* getOrInstantiateHeaderTree(const GEventDataCollectionHeader* gheader, std::shared_ptr<GLogger>& log);
	GRootTree* getOrInstantiateTrueInfoDataTree(const string& detectorName, const GTrueInfoData* gdata, std::shared_ptr<GLogger>& log);
	GRootTree* getOrInstantiateDigitizedDataTree(const string& detectorName, const GDigitizedData* gdata, std::shared_ptr<GLogger>& log);

	// instantiated (and their variable maps) during the first event in startEvent
	map<string, GRootTree*>* gRootTrees;

};
