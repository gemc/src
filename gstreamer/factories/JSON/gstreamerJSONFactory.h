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
 * \file gstreamerJSONFactory.h
 * \brief JSON streamer plugin declarations.
 * \ingroup gstreamer_plugin_json_api
 */

/**
 * \class GstreamerJsonFactory
 * \ingroup gstreamer_plugin_json_api
 * \brief JSON plugin writing structured event or frame output into a single JSON document.
 *
 * The plugin writes one top-level JSON object per output file. Depending on the active stream type,
 * the document contains either:
 * - an \c "events" array
 * - or a \c "frames" array
 *
 * Event objects are assembled incrementally following the publish sequence driven by the base class.
 * This allows the plugin to receive event header, detector true-information, and detector digitized
 * content in a well-defined order and serialize them into one structured JSON entry.
 *
 * The plugin performs lightweight JSON escaping internally and intentionally avoids introducing an
 * external JSON dependency.
 *
 * Threading model:
 * - one instance per worker thread is the intended usage
 * - copy and move are disabled to prevent accidental sharing of stream and assembly state
 */
class GstreamerJsonFactory : public GStreamer {
public:
	/// \brief Inherit the constructor taking the parsed options container.
	using GStreamer::GStreamer;

	GstreamerJsonFactory(const GstreamerJsonFactory&)            = delete;
	GstreamerJsonFactory& operator=(const GstreamerJsonFactory&) = delete;
	GstreamerJsonFactory(GstreamerJsonFactory&&)                 = delete;
	GstreamerJsonFactory& operator=(GstreamerJsonFactory&&)      = delete;

private:
	/**
	 * \brief Open the JSON output file for this plugin instance.
	 *
	 * The file is truncated on open. The top-level JSON object is not written immediately because
	 * the plugin waits until it knows whether the output will contain events or frames.
	 *
	 * \return \c true when the file is ready for writing, \c false otherwise.
	 */
	bool openConnection() override;

	/**
	 * \brief Close the JSON output file after finishing the top-level JSON structure.
	 *
	 * \return \c true on success, \c false otherwise.
	 */
	bool closeConnectionImpl() override;

	/**
	 * \brief Begin assembly of one JSON event object.
	 *
	 * The implementation resets all event-local assembly state and starts the event object.
	 *
	 * \param event_data Event collection being serialized.
	 * \return \c true on success, \c false otherwise.
	 */
	bool startEventImpl(const std::shared_ptr<GEventDataCollection>& event_data) override;

	/**
	 * \brief Finalize and write the current JSON event object.
	 *
	 * \param event_data Event collection being serialized.
	 * \return \c true on success, \c false otherwise.
	 */
	bool endEventImpl(const std::shared_ptr<GEventDataCollection>& event_data) override;

	/**
	 * \brief Append event header information to the current JSON event object.
	 *
	 * \param gevent_header Event header for the current event.
	 * \return \c true on success, \c false otherwise.
	 */
	bool publishEventHeaderImpl(const std::unique_ptr<GEventHeader>& gevent_header) override;

	/**
	 * \brief Append one detector true-information block to the current JSON event object.
	 *
	 * \param detectorName Detector name used as the JSON key.
	 * \param trueInfoData True-information hits for that detector.
	 * \return \c true on success, \c false otherwise.
	 */
	bool publishEventTrueInfoDataImpl(const std::string& detectorName,
	                                  const std::vector<const GTrueInfoData*>& trueInfoData) override;

	/**
	 * \brief Append one detector digitized block to the current JSON event object.
	 *
	 * \param detectorName Detector name used as the JSON key.
	 * \param digitizedData Digitized hits for that detector.
	 * \return \c true on success, \c false otherwise.
	 */
	bool publishEventDigitizedDataImpl(const std::string& detectorName,
	                                   const std::vector<const GDigitizedData*>& digitizedData) override;

	/**
	 * \brief Begin assembly of one JSON frame record.
	 *
	 * \param frameRunData Frame collection associated with the record.
	 * \return \c true on success, \c false otherwise.
	 */
	bool startStreamImpl(const GFrameDataCollection* frameRunData) override;

	/**
	 * \brief Finalize and write the current JSON frame record.
	 *
	 * \param frameRunData Frame collection associated with the record.
	 * \return \c true on success, \c false otherwise.
	 */
	bool endStreamImpl(const GFrameDataCollection* frameRunData) override;

	/**
	 * \brief Append frame header information to the current JSON frame record.
	 *
	 * \param gframeHeader Frame header associated with the record.
	 * \return \c true on success, \c false otherwise.
	 */
	bool publishFrameHeaderImpl(const GFrameHeader* gframeHeader) override;

	/**
	 * \brief Append frame payload information to the current JSON frame record.
	 *
	 * \param payload Payload vector associated with the record.
	 * \return \c true on success, \c false otherwise.
	 */
	bool publishPayloadImpl(const std::vector<GIntegralPayload*>* payload) override;

private:
	/// \brief Output stream bound to the JSON file for this plugin instance.
	std::ofstream ofile;

	/// \brief Tracks whether the top-level JSON object has already been started.
	bool is_file_initialized = false;

	/// \brief Tracks whether at least one top-level entry has been written into the active array.
	bool wrote_first_top_level_entry = false;

	/// \brief Active top-level stream type, typically \c "event" or \c "stream".
	std::string top_level_type;

	/// \brief Tracks whether the plugin is currently assembling an event object.
	bool is_building_event = false;

	/// \brief String buffer holding the current event JSON object under construction.
	std::ostringstream current_event;

	/// \brief Tracks whether the current event already contains header content.
	bool current_event_has_header = false;

	/// \brief Tracks whether the current event already contains at least one detector block.
	bool current_event_has_any_detector = false;

	/// \brief Tracks whether the plugin is currently assembling a frame object.
	bool is_building_frame = false;

	/// \brief String buffer holding the current frame JSON object under construction.
	std::ostringstream current_frame;

	/// \brief Tracks whether the current frame already contains header content.
	bool current_frame_has_header = false;

	/// \brief Tracks whether the current frame already contains payload content.
	bool current_frame_has_payload = false;

	/// \brief Cached timestamp from the current event header.
	std::string timestamp;

	/// \brief Cached event number for the current event object.
	int event_number = -1;

	/// \brief Cached thread id from the current event header.
	int thread_id    = -1;

private:
	/**
	 * \brief Return the final JSON output filename for this plugin instance.
	 *
	 * \return Base output name plus the \c ".json" extension.
	 */
	[[nodiscard]] std::string filename() const override { return gstreamer_definitions.rootname + ".json"; }

private:
	// Private helper utilities. These remain undocumented by cross-reference on purpose.
	static std::string jsonEscape(const std::string& s);
	void ensureFileInitializedForType(const std::string& type);
	void writeTopLevelEntry(const std::string& entry_json);
	void closeTopLevelObjectIfNeeded();
};