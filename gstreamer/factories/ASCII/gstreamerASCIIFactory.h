#pragma once

// gstreamer
#include "gstreamer.h"

// c++
#include <fstream>

/**
 * \class GstreamerTextFactory
 * \brief ASCII (human-readable) gstreamer plugin writing event and frame streams to a text file.
 *
 * This plugin serializes:
 * - Event headers and event hit banks (true info and digitized data) in a structured text format.
 * - Frame stream content (frame header and payload) in a basic text representation.
 *
 * Threading:
 * - Intended usage is one instance per worker thread.
 * - Copy/move is disabled to prevent accidental sharing of the underlying file stream.
 *
 * Filename:
 * - Derived from \ref GStreamer::define_gstreamer "define_gstreamer()" via \ref GStreamerDefinition::rootname
 *   plus the \c ".txt" extension.
 */
class GstreamerTextFactory : public GStreamer
{
public:
	/// \brief Inherit the base constructor taking (const std::shared_ptr<GOptions>&).
	using GStreamer::GStreamer;

	// One instance per thread: forbid copy/move to prevent accidental sharing.
	GstreamerTextFactory(const GstreamerTextFactory&)            = delete;
	GstreamerTextFactory& operator=(const GstreamerTextFactory&) = delete;
	GstreamerTextFactory(GstreamerTextFactory&&)                 = delete;
	GstreamerTextFactory& operator=(GstreamerTextFactory&&)      = delete;

private:
	/// \brief Open the output file stream for this thread.
	bool openConnection() override;

	/// \brief Close the output file stream (after flushing any buffered events).
	bool closeConnectionImpl() override;

	// Event stream hooks.
	bool startEventImpl([[maybe_unused]] const std::shared_ptr<GEventDataCollection>& event_data) override;
	bool endEventImpl([[maybe_unused]] const std::shared_ptr<GEventDataCollection>& event_data) override;
	bool publishEventHeaderImpl([[maybe_unused]] const std::unique_ptr<GEventHeader>& gevent_header) override;
	bool publishEventTrueInfoDataImpl(const std::string&                       detectorName,
	                                  const std::vector<const GTrueInfoData*>& trueInfoData) override;
	bool publishEventDigitizedDataImpl(const std::string&                        detectorName,
	                                   const std::vector<const GDigitizedData*>& digitizedData) override;

	// Frame stream hooks.
	bool startStreamImpl(const GFrameDataCollection* frameRunData) override;
	bool endStreamImpl(const GFrameDataCollection* frameRunData) override;
	bool publishFrameHeaderImpl(const GFrameHeader* gframeHeader) override;
	bool publishPayloadImpl(const std::vector<GIntegralPayload*>* payload) override;

	/// \brief Output stream for the ASCII file.
	std::ofstream ofile;

	/// \brief Return the output filename for this plugin instance.
	[[nodiscard]] std::string filename() const override { return gstreamer_definitions.rootname + ".txt"; }
};
