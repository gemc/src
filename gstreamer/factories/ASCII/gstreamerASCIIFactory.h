#pragma once

// gstreamer
#include "gstreamer.h"

// c++
#include <fstream>

/**
 * \file gstreamerASCIIFactory.h
 * \brief ASCII streamer plugin declarations.
 * \ingroup gstreamer_plugin_ascii_api
 */

/**
 * \class GstreamerTextFactory
 * \ingroup gstreamer_plugin_ascii_api
 * \brief ASCII plugin that writes human-readable event, run, and frame content to a text file.
 *
 * This plugin is intended for inspection, debugging, and lightweight text-based output.
 * It serializes the publish sequence into a structured plain-text representation containing:
 * - event boundaries
 * - event header content
 * - detector true-information banks
 * - detector digitized banks
 * - run boundaries and run-level digitized content
 * - frame boundaries, frame headers, and frame payloads
 *
 * Each plugin instance writes a single text file whose final name is derived from the configured
 * \ref GStreamerDefinition root name plus the \c ".txt" extension.
 *
 * Threading model:
 * - one instance per worker thread is the intended usage
 * - copy and move are disabled so the underlying file stream cannot be shared accidentally
 */
class GstreamerTextFactory : public GStreamer
{
public:
	/// \brief Inherit the constructor taking the parsed options container.
	using GStreamer::GStreamer;

	GstreamerTextFactory(const GstreamerTextFactory&)            = delete;
	GstreamerTextFactory& operator=(const GstreamerTextFactory&) = delete;
	GstreamerTextFactory(GstreamerTextFactory&&)                 = delete;
	GstreamerTextFactory& operator=(GstreamerTextFactory&&)      = delete;

private:
	/**
	 * \brief Open the text output file for this streamer instance.
	 *
	 * The file is opened in truncate mode so each execution starts with a fresh text output.
	 *
	 * \return \c true if the file is available for writing, \c false otherwise.
	 */
	bool openConnection() override;

	/**
	 * \brief Close the text output file after any pending buffered events have been published.
	 *
	 * \return \c true on success, \c false otherwise.
	 */
	bool closeConnectionImpl() override;

	/**
	 * \brief Begin one event block in the text output.
	 *
	 * \param event_data Event collection whose header provides the event number.
	 * \return \c true on success, \c false otherwise.
	 */
	bool startEventImpl([[maybe_unused]] const std::shared_ptr<GEventDataCollection>& event_data) override;

	/**
	 * \brief End one event block in the text output.
	 *
	 * \param event_data Event collection whose header provides the event number.
	 * \return \c true on success, \c false otherwise.
	 */
	bool endEventImpl([[maybe_unused]] const std::shared_ptr<GEventDataCollection>& event_data) override;

	/**
	 * \brief Write the event header bank in text form.
	 *
	 * \param gevent_header Event header to serialize.
	 * \return \c true on success, \c false otherwise.
	 */
	bool publishEventHeaderImpl([[maybe_unused]] const std::unique_ptr<GEventHeader>& gevent_header) override;

	/**
	 * \brief Write one detector true-information bank in text form.
	 *
	 * \param detectorName Name of the detector whose hits are being serialized.
	 * \param trueInfoData True-information hits belonging to that detector.
	 * \return \c true on success, \c false otherwise.
	 */
	bool publishEventTrueInfoDataImpl(const std::string& detectorName,
									  const std::vector<const GTrueInfoData*>& trueInfoData) override;

	/**
	 * \brief Write one detector digitized bank in text form.
	 *
	 * \param detectorName Name of the detector whose hits are being serialized.
	 * \param digitizedData Digitized hits belonging to that detector.
	 * \return \c true on success, \c false otherwise.
	 */
	bool publishEventDigitizedDataImpl(const std::string& detectorName,
									   const std::vector<const GDigitizedData*>& digitizedData) override;

	/**
	 * \brief Begin one run block in the text output.
	 *
	 * \param run_data Run collection providing summary information.
	 * \return \c true on success, \c false otherwise.
	 */
	bool startRunImpl([[maybe_unused]] const std::shared_ptr<GRunDataCollection>& run_data) override;

	/**
	 * \brief End one run block in the text output.
	 *
	 * \param run_data Run collection providing the run number.
	 * \return \c true on success, \c false otherwise.
	 */
	bool endRunImpl([[maybe_unused]] const std::shared_ptr<GRunDataCollection>& run_data) override;

	/**
	 * \brief Write one run-level detector digitized bank in text form.
	 *
	 * \param detectorName Name of the detector whose run data are being serialized.
	 * \param digitizedData Run-level digitized data for that detector.
	 * \return \c true on success, \c false otherwise.
	 */
	bool publishRunDigitizedDataImpl(const std::string& detectorName,
									 const std::vector<const GDigitizedData*>& digitizedData) override;

	/**
	 * \brief Begin one frame block in the text output.
	 *
	 * \param frameRunData Frame collection associated with the output record.
	 * \return \c true on success, \c false otherwise.
	 */
	bool startStreamImpl(const GFrameDataCollection* frameRunData) override;

	/**
	 * \brief End one frame block in the text output.
	 *
	 * \param frameRunData Frame collection associated with the output record.
	 * \return \c true on success, \c false otherwise.
	 */
	bool endStreamImpl(const GFrameDataCollection* frameRunData) override;

	/**
	 * \brief Write the frame header in text form.
	 *
	 * \param gframeHeader Frame header to serialize.
	 * \return \c true on success, \c false otherwise.
	 */
	bool publishFrameHeaderImpl(const GFrameHeader* gframeHeader) override;

	/**
	 * \brief Write the frame payload in text form.
	 *
	 * \param payload Frame payload vector to serialize.
	 * \return \c true on success, \c false otherwise.
	 */
	bool publishPayloadImpl(const std::vector<GIntegralPayload*>* payload) override;

	/// \brief Output stream bound to the text file for this plugin instance.
	std::ofstream ofile;

	/**
	 * \brief Return the final output filename for this plugin instance.
	 *
	 * \return Base output name plus the \c ".txt" extension.
	 */
	[[nodiscard]] std::string filename() const override { return gstreamer_definitions.rootname + ".txt"; }
};