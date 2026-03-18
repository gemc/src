#pragma once

// gstreamer
#include "gstreamer.h"

// c++
#include <fstream>

/**
 * \file gstreamerCSVFactory.h
 * \brief CSV streamer plugin declarations.
 * \ingroup gstreamer_plugin_csv_api
 */

/**
 * \class GstreamerCsvFactory
 * \ingroup gstreamer_plugin_csv_api
 * \brief CSV plugin writing flattened per-hit event and run data into two CSV files.
 *
 * This plugin separates true-information and digitized data into distinct output files:
 * - \c "<rootname>_true_info.csv"
 * - \c "<rootname>_digitized.csv"
 *
 * For each file, the first non-empty collection determines the column set. A header row is then
 * emitted once, and every subsequent hit is written as one flattened row.
 *
 * Event-mode rows include:
 * - event number
 * - timestamp
 * - thread id
 * - detector name
 * - hit variables
 *
 * Run-mode digitized rows reuse the same flattened output strategy for run collections.
 *
 * Threading model:
 * - one instance per worker thread is the intended usage
 * - copy and move are disabled to avoid accidental sharing of streams and cached state
 */
class GstreamerCsvFactory : public GStreamer
{
public:
	/// \brief Inherit the constructor taking the parsed options container.
	using GStreamer::GStreamer;

	GstreamerCsvFactory(const GstreamerCsvFactory&)            = delete;
	GstreamerCsvFactory& operator=(const GstreamerCsvFactory&) = delete;
	GstreamerCsvFactory(GstreamerCsvFactory&&)                 = delete;
	GstreamerCsvFactory& operator=(GstreamerCsvFactory&&)      = delete;

private:
	/**
	 * \brief Open both CSV output files used by this plugin instance.
	 *
	 * \return \c true when the files are available for writing, \c false otherwise.
	 */
	bool openConnection() override;

	/**
	 * \brief Close both CSV output files after buffered events have been flushed.
	 *
	 * \return \c true on success, \c false otherwise.
	 */
	bool closeConnectionImpl() override;

	/**
	 * \brief Begin one event publication cycle.
	 *
	 * The implementation caches the current event number so later publish steps can flatten it into
	 * CSV rows.
	 *
	 * \param event_data Event collection being published.
	 * \return \c true on success, \c false otherwise.
	 */
	bool startEventImpl([[maybe_unused]] const std::shared_ptr<GEventDataCollection>& event_data) override;

	/**
	 * \brief End one event publication cycle.
	 *
	 * \param event_data Event collection being published.
	 * \return \c true on success, \c false otherwise.
	 */
	bool endEventImpl([[maybe_unused]] const std::shared_ptr<GEventDataCollection>& event_data) override;

	/**
	 * \brief Cache event header values used later in flattened CSV rows.
	 *
	 * \param gevent_header Event header providing timestamp and thread id.
	 * \return \c true on success, \c false otherwise.
	 */
	bool publishEventHeaderImpl([[maybe_unused]] const std::unique_ptr<GEventHeader>& gevent_header) override;

	/**
	 * \brief Write one detector true-information collection into the true-info CSV file.
	 *
	 * \param detectorName Detector name written into the detector column.
	 * \param trueInfoData True-information hits to flatten into rows.
	 * \return \c true on success, \c false otherwise.
	 */
	bool publishEventTrueInfoDataImpl(const std::string& detectorName,
	                                  const std::vector<const GTrueInfoData*>& trueInfoData) override;

	/**
	 * \brief Write one detector digitized collection into the digitized CSV file.
	 *
	 * \param detectorName Detector name written into the detector column.
	 * \param digitizedData Digitized hits to flatten into rows.
	 * \return \c true on success, \c false otherwise.
	 */
	bool publishEventDigitizedDataImpl(const std::string& detectorName,
	                                   const std::vector<const GDigitizedData*>& digitizedData) override;

	/**
	 * \brief Begin one run publication cycle.
	 *
	 * The implementation caches the run identifier for later use in diagnostics and row generation.
	 *
	 * \param run_data Run collection being published.
	 * \return \c true on success, \c false otherwise.
	 */
	bool startRunImpl([[maybe_unused]] const std::shared_ptr<GRunDataCollection>& run_data) override;

	/**
	 * \brief End one run publication cycle.
	 *
	 * \param run_data Run collection being published.
	 * \return \c true on success, \c false otherwise.
	 */
	bool endRunImpl([[maybe_unused]] const std::shared_ptr<GRunDataCollection>& run_data) override;

	/**
	 * \brief Write one detector run-level digitized collection into the digitized CSV file.
	 *
	 * \param detectorName Detector name written into the detector column.
	 * \param digitizedData Run-level digitized hits to flatten into rows.
	 * \return \c true on success, \c false otherwise.
	 */
	bool publishRunDigitizedDataImpl(const std::string& detectorName,
									 const std::vector<const GDigitizedData*>& digitizedData) override;

	/**
	 * \brief Begin one frame-stream publication cycle.
	 *
	 * This plugin currently implements the frame API as a no-op placeholder.
	 *
	 * \param frameRunData Frame collection associated with the request.
	 * \return Always \c true.
	 */
	bool startStreamImpl(const GFrameDataCollection* frameRunData) override;

	/**
	 * \brief End one frame-stream publication cycle.
	 *
	 * \param frameRunData Frame collection associated with the request.
	 * \return Always \c true.
	 */
	bool endStreamImpl(const GFrameDataCollection* frameRunData) override;

	/**
	 * \brief Accept a frame header publish request.
	 *
	 * This plugin currently does not serialize frame headers.
	 *
	 * \param gframeHeader Frame header associated with the request.
	 * \return Always \c true.
	 */
	bool publishFrameHeaderImpl(const GFrameHeader* gframeHeader) override;

	/**
	 * \brief Accept a frame payload publish request.
	 *
	 * This plugin currently does not serialize frame payloads.
	 *
	 * \param payload Frame payload associated with the request.
	 * \return Always \c true.
	 */
	bool publishPayloadImpl(const std::vector<GIntegralPayload*>* payload) override;

	/// \brief Output stream for the true-information CSV file.
	std::ofstream ofile_true_info;

	/// \brief Output stream for the digitized CSV file.
	std::ofstream ofile_digitized;

	/**
	 * \brief Return the generic filename base for this plugin.
	 *
	 * This method is required by the base class but is not used directly because the plugin writes
	 * two separate CSV files.
	 *
	 * \return Base output name plus \c ".csv".
	 */
	[[nodiscard]] std::string filename() const override { return gstreamer_definitions.rootname + ".csv"; }

	/**
	 * \brief Return the true-information CSV filename.
	 *
	 * \return Base output name plus \c "_true_info.csv".
	 */
	[[nodiscard]] std::string filename_true_info() const { return gstreamer_definitions.rootname + "_true_info.csv"; }

	/**
	 * \brief Return the digitized CSV filename.
	 *
	 * \return Base output name plus \c "_digitized.csv".
	 */
	[[nodiscard]] std::string filename_digitized() const { return gstreamer_definitions.rootname + "_digitized.csv"; }

	/// \brief Tracks whether the true-information CSV header row has already been emitted.
	bool is_first_event_with_truedata = false;

	/// \brief Tracks whether the digitized CSV header row has already been emitted.
	bool is_first_event_with_digidata = false;

	/// \brief Cached event timestamp copied from the most recent event header.
	std::string timestamp;

	/// \brief Cached event number copied at the start of the most recent event publish cycle.
	int event_number;

	/// \brief Cached run number copied at the start of the most recent run publish cycle.
	int runId;

	/// \brief Cached thread id copied from the most recent event header.
	int thread_id;
};