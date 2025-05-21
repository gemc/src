#pragma once

// gstreamer
#include "gstreamer.h"

// c++
#include <fstream>

class GstreamerTextFactory : public GStreamer {
public:
	GstreamerTextFactory() = default;

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
	bool publishEventTrueInfoData(std::string                        detectorName,
	                              const std::vector<GTrueInfoData*>* trueInfoData,
	                              std::shared_ptr<GLogger>&     log) override;
	bool publishEventDigitizedData(std::string                         detectorName,
	                               const std::vector<GDigitizedData*>* digitizedData,
	                               std::shared_ptr<GLogger>&      log) override;

	// frame streams
	bool startStream(const GFrameDataCollection* frameRunData, const std::shared_ptr<GLogger>& log) override;
	bool endStream(const GFrameDataCollection* frameRunData, const std::shared_ptr<GLogger>& log) override;
	bool publishFrameHeader(const GFrameDataCollectionHeader* gframeHeader, const std::shared_ptr<GLogger>& log) override;
	bool publishPayload(const std::vector<GIntegralPayload*>* payload, const std::shared_ptr<GLogger>& log) override;

private:
	std::ofstream* ofile = nullptr;
};
