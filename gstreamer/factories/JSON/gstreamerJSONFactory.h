#pragma once

// gstreamer
#include "gstreamer.h"
#include "gstreamerConventions.h"

// c++
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

/**
 * \class GstreamerJsonFactory
 *
 * \brief Writes gstreamer output to a JSON file.
 *
 * This factory writes either:
 * - **event streams**: a JSON object containing an `"events"` array
 * - **frame streams**: a JSON object containing a `"frames"` array
 *
 * The top-level structure is always a single JSON object:
 * \code
 * {
 *   "type": "event",
 *   "events": [
 *     { ... event 1 ... },
 *     { ... event 2 ... }
 *   ]
 * }
 * \endcode
 *
 * \note The gstreamer core publishes event data in this order:
 * - \c startEventImpl(...)
 * - \c publishEventHeaderImpl(...)
 * - \c publishEventTrueInfoDataImpl(...) for each detector
 * - \c publishEventDigitizedDataImpl(...) for each detector
 * - \c endEventImpl(...)
 *
 * The JSON factory relies on that ordering to build a well-formed event object incrementally.
 *
 * \note For hit objects, this factory uses:
 *
 * - getIdentityString() for the hit address
 * - getDoubleVariablesMap() and
 * - getStringVariablesMap() for true info variables
 * - getIdentityString() for the hit address
 * - getIntObservablesMap(0) and
 * - getDblObservablesMap(0) for digitized observables
 *
 */
class GstreamerJsonFactory : public GStreamer {
public:
	/// Inherit the base constructor: \c GStreamer(const std::shared_ptr<GOptions>&).
	using GStreamer::GStreamer;

	/// One instance per thread: forbid copy/move to prevent accidental sharing of output streams.
	GstreamerJsonFactory(const GstreamerJsonFactory&)            = delete;
	GstreamerJsonFactory& operator=(const GstreamerJsonFactory&) = delete;
	GstreamerJsonFactory(GstreamerJsonFactory&&)                 = delete;
	GstreamerJsonFactory& operator=(GstreamerJsonFactory&&)      = delete;

private:
	/**
	 * \brief Opens the JSON output file for the current thread.
	 *
	 * The file is truncated on open. The factory always writes a single top-level JSON object.
	 * \return \c true if the file is open (or already open), \c false otherwise.
	 */
	bool openConnection() override;

	/**
	 * \brief Closes the JSON output file.
	 *
	 * This method flushes any buffered events (via \c flushEventBuffer()) and then
	 * writes the closing JSON brackets for the top-level object.
	 *
	 * \return \c true on success, \c false otherwise.
	 */
	bool closeConnectionImpl() override;

	/**
	 * \brief Starts a new event object.
	 *
	 * This initializes internal state for event assembly. The event is written to the
	 * `"events"` array when \c endEventImpl(...) is called.
	 *
	 * \param event_data Event data collection; must not be null and must have a header.
	 * \return \c true to continue publishing, \c false on fatal error.
	 */
	bool startEventImpl(const std::shared_ptr<GEventDataCollection>& event_data) override;

	/**
	 * \brief Finalizes and writes the current event object into the JSON file.
	 *
	 * \param event_data Event data collection; used only for diagnostics and consistency.
	 * \return \c true if the event was written, \c false otherwise.
	 */
	bool endEventImpl(const std::shared_ptr<GEventDataCollection>& event_data) override;

	/**
	 * \brief Stores header fields for the current event.
	 *
	 * \param gevent_header Event header; must not be null.
	 * \return \c true if header fields were stored, \c false otherwise.
	 */
	bool publishEventHeaderImpl(const std::unique_ptr<GEventHeader>& gevent_header) override;

	/**
	 * \brief Appends true information hits for a given detector to the current event.
	 *
	 * \param detectorName Sensitive detector name (used as key in the `"detectors"` object).
	 * \param trueInfoData Array of raw pointers to hit objects (read-only).
	 * \return \c true if the detector block was appended, \c false otherwise.
	 */
	bool publishEventTrueInfoDataImpl(const std::string& detectorName,
	                                  const std::vector<const GTrueInfoData*>& trueInfoData) override;

	/**
	 * \brief Appends digitized hits for a given detector to the current event.
	 *
	 * \param detectorName Sensitive detector name (used as key in the `"detectors"` object).
	 * \param digitizedData Array of raw pointers to hit objects (read-only).
	 * \return \c true if the detector block was appended, \c false otherwise.
	 */
	bool publishEventDigitizedDataImpl(const std::string& detectorName,
	                                   const std::vector<const GDigitizedData*>& digitizedData) override;

	/**
	 * \brief Starts a new frame-stream record.
	 *
	 * \note The gstreamer core flushes the event buffer before starting a stream
	 * (\c GStreamer::startStream), so a single file will not mix buffered events
	 * with frames.
	 *
	 * \param frameRunData Frame collection; can be null (error will be logged).
	 * \return \c true to continue, \c false otherwise.
	 */
	bool startStreamImpl(const GFrameDataCollection* frameRunData) override;

	/**
	 * \brief Ends the current frame-stream record.
	 * \param frameRunData Frame collection; optional.
	 * \return \c true on success, \c false otherwise.
	 */
	bool endStreamImpl(const GFrameDataCollection* frameRunData) override;

	/**
	 * \brief Publishes frame header information into the current frame record.
	 * \param gframeHeader Frame header; can be null (error will be logged).
	 * \return \c true on success, \c false otherwise.
	 */
	bool publishFrameHeaderImpl(const GFrameHeader* gframeHeader) override;

	/**
	 * \brief Publishes the integral payload for the current frame record.
	 * \param payload Payload vector; can be null (error will be logged).
	 * \return \c true on success, \c false otherwise.
	 */
	bool publishPayloadImpl(const std::vector<GIntegralPayload*>* payload) override;

private:
	// Output stream (one instance per worker thread).
	std::ofstream ofile;

	// True if the file header (opening braces) has been written.
	bool is_file_initialized = false;

	// Tracks whether we've already written at least one entry inside the current top-level array.
	bool wrote_first_top_level_entry = false;

	// Active top-level mode: "event" or "stream".
	std::string top_level_type;

	// Event assembly state.
	bool is_building_event = false;
	std::ostringstream current_event;
	bool current_event_has_header = false;
	bool current_event_has_any_detector = false;

	// Frame assembly state (minimal support).
	bool is_building_frame = false;
	std::ostringstream current_frame;
	bool current_frame_has_header = false;
	bool current_frame_has_payload = false;

	// Cached header fields (used by event writers).
	std::string timestamp;
	int         event_number = -1;
	int         thread_id    = -1;

private:
	[[nodiscard]] std::string filename() const override { return gstreamer_definitions.rootname + ".json"; }

private:
	// Helper utilities (kept private and not cross-referenced).
	static std::string jsonEscape(const std::string& s);
	void              ensureFileInitializedForType(const std::string& type);
	void              writeTopLevelEntry(const std::string& entry_json);
	void              closeTopLevelObjectIfNeeded();
};
