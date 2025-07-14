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
	bool closeConnection() override;

	// event streams
	// start and end each event
	bool startEventImpl(const GEventDataCollection* eventData) override;
	bool endEventImpl(const GEventDataCollection* eventData) override;

	// write the header
	bool publishEventHeaderImpl(const GEventDataCollectionHeader* gheader) override;

	// vector index is hit number
	bool publishEventTrueInfoDataImpl(std::string                        detectorName,
	                                  const std::vector<GTrueInfoData*>* trueInfoData) override;
	bool publishEventDigitizedDataImpl(std::string                         detectorName,
	                                   const std::vector<GDigitizedData*>* digitizedData) override;

	// frame streams
	bool startStreamImpl(const GFrameDataCollection* frameRunData) override;
	bool endStreamImpl(const GFrameDataCollection* frameRunData) override;
	bool publishFrameHeaderImpl(const GFrameDataCollectionHeader* gframeHeader) override;
	bool publishPayloadImpl(const std::vector<GIntegralPayload*>* payload) override;

private:
	std::ofstream* ofile = nullptr;
	std::string    filename() const override { return gstreamer_definitions.rootname + ".txt"; }
};
