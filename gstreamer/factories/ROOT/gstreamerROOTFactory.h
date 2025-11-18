#pragma once

// gstreamer
#include "gstreamer.h"
#include "gRootTree.h"

// ROOT
#include "TFile.h"

class GstreamerRootFactory : public GStreamer {
public:
	//GstreamerRootFactory() = default;

	// inherit the base (const std::shared_ptr<GOptions>&) ctor
	using GStreamer::GStreamer;

private:
	// open and close the output media
	bool openConnection() override;
	bool closeConnectionImpl() override;

	// event streams
	// start and end each event
	bool startEventImpl([[maybe_unused]] const std::shared_ptr<GEventDataCollection>& event_data) override;
	bool endEventImpl([[maybe_unused]] const std::shared_ptr<GEventDataCollection>& event_data) override;

	// write the header
	bool publishEventHeaderImpl([[maybe_unused]] const std::unique_ptr<GEventHeader>& gevent_header) override;

	// vector index is hit number
	bool publishEventTrueInfoDataImpl(const std::string& detectorName, const std::vector<const GTrueInfoData*>& trueInfoData) override;
	bool publishEventDigitizedDataImpl(const std::string& detectorName, const std::vector<const GDigitizedData*>& digitizedData) override;

	// frame streams
	bool startStreamImpl(const GFrameDataCollection* frameRunData) override;
	bool endStreamImpl(const GFrameDataCollection* frameRunData) override;
	bool publishFrameHeaderImpl(const GFrameHeader* gframeHeader) override;
	bool publishPayloadImpl(const std::vector<GIntegralPayload*>* payload) override;

	// returning raw pointers for access
	const std::unique_ptr<GRootTree>& getOrInstantiateHeaderTree([[maybe_unused]] const std::unique_ptr<GEventHeader>& event_header);
	const std::unique_ptr<GRootTree>& getOrInstantiateTrueInfoDataTree(const std::string& treeName, const GTrueInfoData* gdata);
	const std::unique_ptr<GRootTree>& getOrInstantiateDigitizedDataTree(const std::string& treeName, const GDigitizedData* gdata);

	// key is detectr prefix and name for true info or digitized data, or "header" for the header tree
	std::unordered_map<std::string, std::unique_ptr<GRootTree>> gRootTrees;

	std::unique_ptr<TFile> rootfile; // ROOT file pointer

	[[nodiscard]] std::string filename() const override { return gstreamer_definitions.rootname + ".root"; }

};
