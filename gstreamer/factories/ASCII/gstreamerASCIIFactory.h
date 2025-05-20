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
	bool publishEventTrueInfoData(string                        detectorName,
	                              const vector<GTrueInfoData*>* trueInfoData,
	                              std::shared_ptr<GLogger>&     log) override;
	bool publishEventDigitizedData(string                         detectorName,
	                               const vector<GDigitizedData*>* digitizedData,
	                               std::shared_ptr<GLogger>&      log) override;

	// frame streams
	bool startStream(const GFrameDataCollection* frameRunData, const std::shared_ptr<GLogger>& log) override;
	bool endStream(const GFrameDataCollection* frameRunData, const std::shared_ptr<GLogger>& log) override;
	bool publishFrameHeader(const GFrameDataCollectionHeader* gframeHeader, const std::shared_ptr<GLogger>& log) override;
	bool publishPayload(const vector<GIntegralPayload*>* payload, const std::shared_ptr<GLogger>& log) override;

private:
	std::ofstream* ofile = nullptr;
};
