#pragma once

// gstreamer
#include "gstreamer.h"
#include "gstreamerConventions.h"

// c++
#include <fstream>

class GstreamerTextFactory : public GStreamer {
public:
	GstreamerTextFactory() = default;

	~GstreamerTextFactory() override { log->debug(NORMAL, "~GstreamerTextFactory"); }

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
	// bool publishEventTrueInfoDataImpl(std::string                        detectorName,
	//                                   const std::vector<GTrueInfoData*>* trueInfoData) override;
	// bool publishEventDigitizedDataImpl(std::string                         detectorName,
	//                                    const std::vector<GDigitizedData*>* digitizedData) override;

	// frame streams
	bool startStreamImpl(const GFrameDataCollection* frameRunData) override;
	bool endStreamImpl(const GFrameDataCollection* frameRunData) override;
	bool publishFrameHeaderImpl(const GFrameHeader* gframeHeader) override;
	bool publishPayloadImpl(const std::vector<GIntegralPayload*>* payload) override;

private:
	std::ofstream* ofile = nullptr;
	std::string    filename() const override { return gstreamer_definitions.rootname + ".txt"; }
};
