#pragma once

// gstreamer
#include "gstreamer.h"

// c++
#include <fstream>

class GstreamerCsvFactory : public GStreamer {
public:
	// inherit the base (const std::shared_ptr<GOptions>&) ctor
	using GStreamer::GStreamer;

	// One instance per thread: forbid copy/move to prevent accidental sharing
	GstreamerCsvFactory(const GstreamerCsvFactory&)            = delete;
	GstreamerCsvFactory& operator=(const GstreamerCsvFactory&) = delete;
	GstreamerCsvFactory(GstreamerCsvFactory&&)                 = delete;
	GstreamerCsvFactory& operator=(GstreamerCsvFactory&&)      = delete;

private:
	// open and close the output media
	bool openConnection() override;
	bool closeConnectionImpl() override;

	// event streams
	// start and end each event
	bool startEventImpl([[maybe_unused]] const std::shared_ptr<GEventDataCollection>& event_data) override;
	bool endEventImpl([[maybe_unused]] const std::shared_ptr<GEventDataCollection>& event_data) override;

	// write the header
	bool publishEventHeaderImpl([[maybe_unused]] const std::unique_ptr<GEventHeader>& gheader) override;

	// vector index is hit number
	bool publishEventTrueInfoDataImpl(const std::string& detectorName, const std::vector<const GTrueInfoData*>& trueInfoData) override;
	bool publishEventDigitizedDataImpl(const std::string& detectorName, const std::vector<const GDigitizedData*>& digitizedData) override;

	// frame streams
	bool startStreamImpl(const GFrameDataCollection* frameRunData) override;
	bool endStreamImpl(const GFrameDataCollection* frameRunData) override;
	bool publishFrameHeaderImpl(const GFrameHeader* gframeHeader) override;
	bool publishPayloadImpl(const std::vector<GIntegralPayload*>* payload) override;

	std::ofstream ofile;

	[[nodiscard]] std::string filename() const override { return gstreamer_definitions.rootname + ".txt"; }
};
