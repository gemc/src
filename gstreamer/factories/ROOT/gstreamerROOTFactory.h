#pragma once

// gstreamer
#include "gstreamer.h"
#include "gRootTree.h"

// ROOT
#include "TFile.h"

class GstreamerRootFactory : public GStreamer {
public:
	GstreamerRootFactory() = default;

// 	~GstreamerRootFactory() override {
// //		log->debug(NORMAL, "~GstreamerRootFactory");
// 	}

private:
	// open and close the output media
	bool openConnection() override;
	bool closeConnection() override;

	// event streams
	// start and end each event
	bool startEventImpl(const GEventDataCollection* eventData) override;
	bool endEventImpl(const GEventDataCollection* eventData) override;

	// write the header
	bool publishEventHeaderImpl(const GEventDataCollectionHeader* gheader) override;

	// vector index is hit number
	bool publishEventTrueInfoDataImpl(std::string detectorName, const std::vector<GTrueInfoData*>* trueInfoData) override;
	bool publishEventDigitizedDataImpl(std::string detectorName, const std::vector<GDigitizedData*>* digitizedData) override;

	// frame streams
	bool startStreamImpl(const GFrameDataCollection* frameRunData) override;
	bool endStreamImpl(const GFrameDataCollection* frameRunData) override;
	bool publishFrameHeaderImpl(const GFrameDataCollectionHeader* gframeHeader) override;
	bool publishPayloadImpl(const std::vector<GIntegralPayload*>* payload) override;

private:

	// returning raw pointers for access
	GRootTree* getOrInstantiateHeaderTree(const GEventDataCollectionHeader* gheader);
	GRootTree* getOrInstantiateTrueInfoDataTree(const std::string& detectorName, const GTrueInfoData* gdata);
	GRootTree* getOrInstantiateDigitizedDataTree(const std::string& detectorName, const GDigitizedData* gdata);

	// key is detectr name + prefix for true info or digitized data, or "header" for the header tree
	std::unordered_map<std::string, std::unique_ptr<GRootTree>> gRootTrees;
	std::unique_ptr<TFile> rootfile{}; // ROOT file pointer

	[[nodiscard]] std::string filename() const override { return gstreamer_definitions.rootname + ".root"; }

};
