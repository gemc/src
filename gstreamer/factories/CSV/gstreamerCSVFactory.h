#pragma once

// gstreamer
#include "gstreamer.h"

// c++
#include <fstream>

/**
 * \class GstreamerCsvFactory
 * \brief CSV gstreamer plugin writing flattened per-hit rows for true and digitized data.
 *
 * Output model:
 * - Two CSV files are produced per streamer instance:
 *   - \c "<rootname>_true_info.csv"
 *   - \c "<rootname>_digitized.csv"
 * - A header row is emitted the first time a non-empty hit vector is encountered.
 * - Each subsequent hit is written as a single row containing:
 *   - event number, timestamp, thread id, detector name
 *   - then hit variables (string/double for true info, int/double for digitized)
 *
 * Threading:
 * - Intended usage is one instance per worker thread.
 * - Copy/move is disabled to prevent accidental sharing of file streams and internal state.
 */
class GstreamerCsvFactory : public GStreamer
{
public:
	/// \brief Inherit the base constructor taking (const std::shared_ptr<GOptions>&).
	using GStreamer::GStreamer;

	// One instance per thread: forbid copy/move to prevent accidental sharing.
	GstreamerCsvFactory(const GstreamerCsvFactory&)            = delete;
	GstreamerCsvFactory& operator=(const GstreamerCsvFactory&) = delete;
	GstreamerCsvFactory(GstreamerCsvFactory&&)                 = delete;
	GstreamerCsvFactory& operator=(GstreamerCsvFactory&&)      = delete;

private:
	/// \brief Open both CSV output files for this thread.
	bool openConnection() override;

	/// \brief Close both CSV output files (after flushing buffered events).
	bool closeConnectionImpl() override;

	// Event stream hooks.
	bool startEventImpl([[maybe_unused]] const std::shared_ptr<GEventDataCollection>& event_data) override;
	bool endEventImpl([[maybe_unused]] const std::shared_ptr<GEventDataCollection>& event_data) override;
	bool publishEventHeaderImpl([[maybe_unused]] const std::unique_ptr<GEventHeader>& gevent_header) override;
	bool publishEventTrueInfoDataImpl(const std::string&                       detectorName,
	                                  const std::vector<const GTrueInfoData*>& trueInfoData) override;
	bool publishEventDigitizedDataImpl(const std::string&                        detectorName,
	                                   const std::vector<const GDigitizedData*>& digitizedData) override;

	// Frame stream hooks (present for interface completeness; currently implemented as no-ops).
	bool startStreamImpl(const GFrameDataCollection* frameRunData) override;
	bool endStreamImpl(const GFrameDataCollection* frameRunData) override;
	bool publishFrameHeaderImpl(const GFrameHeader* gframeHeader) override;
	bool publishPayloadImpl(const std::vector<GIntegralPayload*>* payload) override;

	/// \brief True info CSV file stream.
	std::ofstream ofile_true_info;

	/// \brief Digitized CSV file stream.
	std::ofstream ofile_digitized;

	/// \brief Base filename for the streamer; not used directly for CSV (two-file scheme).
	[[nodiscard]] std::string filename() const override { return gstreamer_definitions.rootname + ".csv"; }
	// never actually called for CSV

	/// \brief Return the true info filename for this instance.
	[[nodiscard]] std::string filename_true_info() const { return gstreamer_definitions.rootname + "_true_info.csv"; }

	/// \brief Return the digitized filename for this instance.
	[[nodiscard]] std::string filename_digitized() const { return gstreamer_definitions.rootname + "_digitized.csv"; }

	/// \brief Tracks whether the true info header row has been emitted.
	bool is_first_event_with_truedata = false;

	/// \brief Tracks whether the digitized header row has been emitted.
	bool is_first_event_with_digidata = false;

	/// \brief Cached header timestamp for the current event (copied in publishEventHeaderImpl()).
	std::string timestamp;

	/// \brief Cached event number for the current event (copied in startEventImpl()).
	int event_number;

	/// \brief Cached thread id for the current event (copied in publishEventHeaderImpl()).
	int thread_id;
};
