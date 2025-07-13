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
	bool openConnectionImpl() override;
	bool closeConnectionImpl() override;

	// event streams
	// start and end each event
	bool startEventImpl(const GEventDataCollection* eventData) override;
	bool endEventImpl(const GEventDataCollection* eventData) override;

	// write the header
	bool publishEventHeaderImpl(const GEventDataCollectionHeader* gheader) override;

	// vector index is hit number
	bool publishEventTrueInfoDataImpl(const std::string detectorName, const std::vector<GTrueInfoData*>* trueInfoData) override;
	bool publishEventDigitizedDataImpl(const std::string detectorName, const std::vector<GDigitizedData*>* digitizedData) override;

	// frame streams
	bool startStreamImpl(const GFrameDataCollection* frameRunData) override;
	bool endStreamImpl(const GFrameDataCollection* frameRunData) override;
	bool publishFrameHeaderImpl(const GFrameDataCollectionHeader* gframeHeader) override;
	bool publishPayloadImpl(const std::vector<GIntegralPayload*>* payload) override;

private:
	std::unique_ptr<TFile> rootfile; // ROOT file pointer

	// return the header tree from the map. If it's not there, initialize it.
	// executed at startEvent
	GRootTree* getOrInstantiateHeaderTree(const GEventDataCollectionHeader* gheader);
	GRootTree* getOrInstantiateTrueInfoDataTree(const std::string& detectorName, const GTrueInfoData* gdata);
	GRootTree* getOrInstantiateDigitizedDataTree(const std::string& detectorName, const GDigitizedData* gdata);

	// instantiated (and their variable maps) during the first event in startEvent
	std::map<std::string, GRootTree*>* gRootTrees;

	std::string filename() const override {
		return gstreamer_definitions.rootname + ".root";
	}

	static std::once_flag rootInitFlag;
};
