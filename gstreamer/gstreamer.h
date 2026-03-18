#pragma once

// gstreamer
#include "gstreamer_options.h"
#include "gstreamerConventions.h"

// gemc
#include "event/gEventDataCollection.h"
#include "run/gRunDataCollection.h"
#include "frame/gFrameDataCollection.h"
#include "gfactory.h"
#include "gbase.h"

// c++
#include <string>
#include <vector>
#include <map>

/**
 * \file gstreamer.h
 * \brief Core streaming interface and helper utilities for the gstreamer module.
 * \ingroup gstreamer_core_api
 */

/**
 * \class GStreamer
 * \ingroup gstreamer_core_api
 * \brief Abstract base class for all gstreamer output plugins.
 *
 * \details
 * A concrete streamer is responsible for serializing GEMC data collections into a specific
 * backend format. The base class centralizes the common workflow:
 * - connection lifecycle management
 * - event buffering
 * - ordered publish sequences for event, run, and frame data
 * - logger setup and configuration propagation
 *
 * Concrete plugins override only the protected implementation hooks they support.
 *
 * \section gstreamer_class_lifecycle Lifecycle
 * A typical plugin instance follows this lifecycle:
 * - construct the concrete streamer
 * - bind one output definition through \ref define_gstreamer "define_gstreamer()"
 * - load runtime settings through \ref set_loggers "set_loggers()"
 * - call \ref openConnection "openConnection()"
 * - publish run, event, or frame data
 * - call \ref closeConnection "closeConnection()"
 *
 * For event-based output, publication is buffered. Events are accumulated in memory and written
 * in batches when the configured buffer threshold is reached or when the connection is closed.
 *
 * \section gstreamer_class_buffering Buffering model
 * Event publication uses an internal buffer of
 * \c std::shared_ptr<GEventDataCollection>. This design ensures that all event-owned objects remain
 * alive for the full duration of a flush. During the flush, the base class extracts raw pointers
 * from the detector collections and passes them to plugin hooks as read-only views.
 *
 * The buffer is flushed when:
 * - the number of queued events reaches \ref bufferFlushLimit
 * - \ref closeConnection "closeConnection()" is called
 * - \ref startStream "startStream()" is called, to avoid mixing buffered event data with frame data
 *
 * \section gstreamer_class_threading Threading expectations
 * The class itself does not perform external synchronization. The intended usage pattern is one
 * streamer instance per worker thread. The helper \ref gstreamer::gstreamersMapPtr "gstreamersMapPtr()"
 * follows that model by creating a per-thread map of streamer objects.
 *
 * Sharing one streamer instance across threads is only safe if the derived plugin implements its
 * own synchronization policy.
 *
 * \section gstreamer_class_factory Plugin factory symbol
 * Concrete plugins are loaded dynamically and must expose an \c extern "C" factory function named
 * \c GStreamerFactory. The static helper \ref instantiate "instantiate()" resolves that symbol from
 * a dynamic library handle and constructs the plugin object.
 */
class GStreamer : public GBase<GStreamer>
{
public:
	/**
	 * \brief Construct the streamer base and initialize module logging.
	 *
	 * \param g Parsed options container used by the base logger infrastructure.
	 */
	explicit GStreamer(const std::shared_ptr<GOptions>& g) : GBase(g, GSTREAMER_LOGGER) {
	}

	/**
	 * \brief Virtual destructor.
	 *
	 * A virtual destructor is required because streamer instances are manipulated through base-class
	 * pointers while the actual object type is a concrete plugin.
	 */
	virtual ~GStreamer() = default;

	/**
	 * \brief Open the output medium used by this streamer.
	 *
	 * Concrete plugins override this to acquire their backend resources, such as files, sockets,
	 * or format-specific handles.
	 *
	 * \return \c true on success, \c false on failure.
	 */
	[[nodiscard]] virtual bool openConnection() { return false; }

	/**
	 * \brief Close the output medium after flushing buffered events.
	 *
	 * This public wrapper guarantees that pending event data are published before the plugin-specific
	 * close logic executes.
	 *
	 * \return \c true on success, \c false on failure.
	 */
	[[nodiscard]] bool closeConnection() {
		flushEventBuffer();
		return closeConnectionImpl();
	}

	/**
	 * \brief Plugin-specific close implementation hook.
	 *
	 * Concrete plugins override this to release backend resources after the common wrapper has
	 * already handled event-buffer flushing.
	 *
	 * \return \c true on success, \c false on failure.
	 */
	[[nodiscard]] virtual bool closeConnectionImpl() { return false; }

	/**
	 * \brief Queue one event for publication.
	 *
	 * The event is appended to the internal event buffer. Once the number of buffered events reaches
	 * \ref bufferFlushLimit, the base class flushes the entire buffer by calling the event publish
	 * hook sequence on the derived plugin.
	 *
	 * \param event_data Event data collection to publish.
	 */
	void publishEventData(const std::shared_ptr<GEventDataCollection>& event_data);

	/**
	 * \brief Publish one run-level data collection immediately.
	 *
	 * Run data are not buffered. The base class dispatches the run publish sequence directly to the
	 * plugin-specific hooks.
	 *
	 * \param run_data Run data collection to publish.
	 */
	void publishRunData(const std::shared_ptr<GRunDataCollection>& run_data);

	/**
	 * \brief Return the semantic stream type associated with this streamer instance.
	 *
	 * The type comes from the configured \ref GStreamerDefinition and is typically \c "event" or
	 * \c "stream".
	 *
	 * \return Stream type string stored in the current definition.
	 */
	[[nodiscard]] inline std::string getStreamType() const { return gstreamer_definitions.type; }

	/**
	 * \brief Assign the output definition used by this streamer instance.
	 *
	 * The assigned definition determines the format token, base filename, type, and optional
	 * thread-specialized naming.
	 *
	 * \param gstreamerDefinition Streamer definition to bind to this instance.
	 * \param tid Worker thread id used to specialize the filename. The default value keeps the
	 * original name unchanged.
	 */
	inline void define_gstreamer(const GStreamerDefinition& gstreamerDefinition, int tid = -1) {
		gstreamer_definitions = GStreamerDefinition(gstreamerDefinition, tid);
	}

	/**
	 * \brief Return the list of output format tokens supported by the module.
	 *
	 * This list represents the built-in plugin formats recognized by the module-level validation
	 * helpers and option help text.
	 *
	 * \return Reference to the static supported-format list.
	 */
	static const std::vector<std::string>& supported_formats();

	/**
	 * \brief Validate whether a format token is supported.
	 *
	 * Validation is case-insensitive.
	 *
	 * \param format Format token to validate.
	 * \return \c true when the token matches one of the supported formats, \c false otherwise.
	 */
	static bool is_valid_format(const std::string& format);

	/**
	 * \brief Load streamer runtime settings from the parsed options container.
	 *
	 * At present this method configures the event buffer flush limit from the \c ebuffer option.
	 *
	 * \param g Parsed options container supplying module configuration.
	 */
	void set_loggers(const std::shared_ptr<GOptions>& g) {
		bufferFlushLimit = g->getScalarInt("ebuffer");
	}

protected:
	/// \brief Output definition currently bound to this streamer instance.
	GStreamerDefinition gstreamer_definitions;

	/**
	 * \brief Begin publishing one buffered event.
	 *
	 * The wrapper validates the input event and its header, emits debug logging, and delegates the
	 * actual backend-specific behavior to the derived implementation.
	 *
	 * \param event_data Event collection being published.
	 * \return \c true on success, \c false on failure.
	 */
	[[nodiscard]] bool startEvent([[maybe_unused]] const std::shared_ptr<GEventDataCollection>& event_data) {
		if (!event_data) { log->error(ERR_PUBLISH_ERROR, "eventData is null in GStreamer::startEvent"); }
		if (!event_data->getHeader()) {
			log->error(ERR_PUBLISH_ERROR, "event header is null in GStreamer::startEvent");
		}

		log->debug(NORMAL, "GStreamer::startEvent");
		return startEventImpl(event_data);
	}

	/**
	 * \brief Plugin-specific implementation hook called at the start of one event publish sequence.
	 *
	 * \param event_data Event collection being published.
	 * \return \c true on success, \c false on failure.
	 */
	virtual bool startEventImpl([[maybe_unused]] const std::shared_ptr<GEventDataCollection>& event_data) {
		return false;
	}

	/**
	 * \brief End publishing one buffered event.
	 *
	 * \param event_data Event collection being published.
	 * \return \c true on success, \c false on failure.
	 */
	[[nodiscard]] bool endEvent([[maybe_unused]] const std::shared_ptr<GEventDataCollection>& event_data) {
		log->debug(NORMAL, "GStreamer::endEvent");
		return endEventImpl(event_data);
	}

	/**
	 * \brief Plugin-specific implementation hook called at the end of one event publish sequence.
	 *
	 * \param event_data Event collection being published.
	 * \return \c true on success, \c false on failure.
	 */
	virtual bool endEventImpl([[maybe_unused]] const std::shared_ptr<GEventDataCollection>& event_data) {
		return false;
	}

	/**
	 * \brief Publish the event header for the current event sequence.
	 *
	 * The wrapper validates the header pointer, emits debug logging, and delegates serialization to
	 * the plugin implementation.
	 *
	 * \param gevent_header Unique pointer owned by the source event collection.
	 * \return \c true on success, \c false on failure.
	 */
	[[nodiscard]] bool publishEventHeader([[maybe_unused]] const std::unique_ptr<GEventHeader>& gevent_header) {
		if (!gevent_header) { log->error(ERR_PUBLISH_ERROR, "event header is null in GStreamer::publishEventHeader"); }
		log->debug(NORMAL, "GStreamer::publishEventHeader");
		return publishEventHeaderImpl(gevent_header);
	}

	/**
	 * \brief Plugin-specific implementation hook for serializing one event header.
	 *
	 * \param gevent_header Unique pointer owned by the source event collection.
	 * \return \c true on success, \c false on failure.
	 */
	virtual bool publishEventHeaderImpl([[maybe_unused]] const std::unique_ptr<GEventHeader>& gevent_header) {
		return false;
	}

	/**
	 * \brief Publish the true-information hit bank for one detector.
	 *
	 * The vector contains raw pointers into event-owned hit objects. Those objects remain valid for
	 * the duration of the flush because the owning event remains stored in the internal buffer.
	 *
	 * \param detectorName Detector or sensitive-detector name identifying the collection.
	 * \param trueInfoData Raw-pointer view of the detector true-information hits.
	 * \return \c true on success, \c false on failure.
	 */
	[[nodiscard]] bool publishEventTrueInfoData([[maybe_unused]] const std::string& detectorName,
												[[maybe_unused]] const std::vector<const GTrueInfoData*>& trueInfoData) {
		log->debug(NORMAL, "GStreamer::publishEventTrueInfoData for detector ", detectorName);
		return publishEventTrueInfoDataImpl(detectorName, trueInfoData);
	}

	/**
	 * \brief Plugin-specific implementation hook for one detector true-information collection.
	 *
	 * \param detectorName Detector or sensitive-detector name.
	 * \param trueInfoData Raw-pointer view of the detector true-information hits.
	 * \return \c true on success, \c false on failure.
	 */
	virtual bool publishEventTrueInfoDataImpl([[maybe_unused]] const std::string& detectorName,
											  [[maybe_unused]] const std::vector<const GTrueInfoData*>& trueInfoData) {
		return false;
	}

	/**
	 * \brief Publish the digitized hit bank for one detector.
	 *
	 * The vector contains raw pointers into event-owned digitized hit objects and is valid for the
	 * duration of the flush.
	 *
	 * \param detectorName Detector or sensitive-detector name identifying the collection.
	 * \param digitizedData Raw-pointer view of the detector digitized hits.
	 * \return \c true on success, \c false on failure.
	 */
	[[nodiscard]] bool publishEventDigitizedData([[maybe_unused]] const std::string& detectorName,
												 [[maybe_unused]] const std::vector<const GDigitizedData*>& digitizedData) {
		log->debug(NORMAL, "GStreamer::publishEventDigitizedData for detector ", detectorName);
		return publishEventDigitizedDataImpl(detectorName, digitizedData);
	}

	/**
	 * \brief Plugin-specific implementation hook for one detector digitized collection.
	 *
	 * \param detectorName Detector or sensitive-detector name.
	 * \param digitizedData Raw-pointer view of the detector digitized hits.
	 * \return \c true on success, \c false on failure.
	 */
	virtual bool publishEventDigitizedDataImpl([[maybe_unused]] const std::string& detectorName,
											   [[maybe_unused]] const std::vector<const GDigitizedData*>& digitizedData) {
		return false;
	}

	/**
	 * \brief Begin publishing one run-level collection.
	 *
	 * \param run_data Run collection being published.
	 * \return \c true on success, \c false on failure.
	 */
	[[nodiscard]] bool startRun([[maybe_unused]] const std::shared_ptr<GRunDataCollection>& run_data) {
		if (!run_data) { log->error(ERR_PUBLISH_ERROR, "run_data is null in GStreamer::startRun"); }
		if (!run_data->getHeader()) {
			log->error(ERR_PUBLISH_ERROR, "run header is null in GStreamer::startRun");
		}

		log->debug(NORMAL, "GStreamer::startRun");
		return startRunImpl(run_data);
	}

	/**
	 * \brief Plugin-specific implementation hook called at the start of a run publish sequence.
	 *
	 * \param run_data Run collection being published.
	 * \return \c true on success, \c false on failure.
	 */
	virtual bool startRunImpl([[maybe_unused]] const std::shared_ptr<GRunDataCollection>& run_data) {
		return false;
	}

	/**
	 * \brief End publishing one run-level collection.
	 *
	 * \param run_data Run collection being published.
	 * \return \c true on success, \c false on failure.
	 */
	[[nodiscard]] bool endRun([[maybe_unused]] const std::shared_ptr<GRunDataCollection>& run_data) {
		log->debug(NORMAL, "GStreamer::endRun");
		return endRunImpl(run_data);
	}

	/**
	 * \brief Plugin-specific implementation hook called at the end of a run publish sequence.
	 *
	 * \param run_data Run collection being published.
	 * \return \c true on success, \c false on failure.
	 */
	virtual bool endRunImpl([[maybe_unused]] const std::shared_ptr<GRunDataCollection>& run_data) {
		return false;
	}

	/**
	 * \brief Publish the run header for the current run sequence.
	 *
	 * \param run_header Unique pointer owned by the source run collection.
	 * \return \c true on success, \c false on failure.
	 */
	bool publishRunHeader([[maybe_unused]] const std::unique_ptr<GRunHeader>& run_header) {
		log->debug(NORMAL, "GStreamer::publishRunHeader");
		return publishRunHeaderImpl(run_header);
	}

	/**
	 * \brief Plugin-specific implementation hook for serializing one run header.
	 *
	 * \param run_header Unique pointer owned by the source run collection.
	 * \return \c true on success, \c false on failure.
	 */
	virtual bool publishRunHeaderImpl([[maybe_unused]] const std::unique_ptr<GRunHeader>& run_header) {
		return false;
	}

	/**
	 * \brief Publish run-level digitized data for one detector.
	 *
	 * \param detectorName Detector or sensitive-detector name identifying the collection.
	 * \param digitizedData Raw-pointer view of the detector digitized data.
	 * \return \c true on success, \c false on failure.
	 */
	bool publishRunDigitizedData([[maybe_unused]] const std::string& detectorName,
								 [[maybe_unused]] const std::vector<const GDigitizedData*>& digitizedData) {
		log->debug(NORMAL, "GStreamer::publishRunDigitizedData for detector ", detectorName);
		return publishRunDigitizedDataImpl(detectorName, digitizedData);
	}

	/**
	 * \brief Plugin-specific implementation hook for one detector run-level digitized collection.
	 *
	 * \param detectorName Detector or sensitive-detector name.
	 * \param digitizedData Raw-pointer view of the detector digitized data.
	 * \return \c true on success, \c false on failure.
	 */
	virtual bool publishRunDigitizedDataImpl([[maybe_unused]] const std::string& detectorName,
											 [[maybe_unused]] const std::vector<const GDigitizedData*>& digitizedData) {
		return false;
	}

	/**
	 * \brief Begin publishing one frame stream record.
	 *
	 * Pending event data are flushed first so the output sequence remains well defined if a plugin
	 * supports both event and frame publication modes.
	 *
	 * \param frameRunData Frame collection being published.
	 * \return \c true on success, \c false on failure.
	 */
	[[nodiscard]] bool startStream([[maybe_unused]] const GFrameDataCollection* frameRunData) {
		flushEventBuffer();
		log->debug(NORMAL, "GStreamer::startStream");
		return startStreamImpl(frameRunData);
	}

	/**
	 * \brief Plugin-specific implementation hook called at the start of a frame stream record.
	 *
	 * \param frameRunData Frame collection being published.
	 * \return \c true on success, \c false on failure.
	 */
	virtual bool startStreamImpl([[maybe_unused]] const GFrameDataCollection* frameRunData) { return false; }

	/**
	 * \brief Publish one frame header.
	 *
	 * \param gframeHeader Frame header pointer supplied by the caller.
	 * \return \c true on success, \c false on failure.
	 */
	[[nodiscard]] bool publishFrameHeader([[maybe_unused]] const GFrameHeader* gframeHeader) {
		log->debug(NORMAL, "GStreamer::publishFrameHeader");
		return publishFrameHeaderImpl(gframeHeader);
	}

	/**
	 * \brief Plugin-specific implementation hook for serializing one frame header.
	 *
	 * \param gframeHeader Frame header pointer supplied by the caller.
	 * \return \c true on success, \c false on failure.
	 */
	virtual bool publishFrameHeaderImpl([[maybe_unused]] const GFrameHeader* gframeHeader) { return false; }

	/**
	 * \brief Publish one frame payload.
	 *
	 * \param payload Pointer to the frame integral-payload vector.
	 * \return \c true on success, \c false on failure.
	 */
	[[nodiscard]] bool publishPayload([[maybe_unused]] const std::vector<GIntegralPayload*>* payload) {
		log->debug(NORMAL, "GStreamer::publishPayload");
		return publishPayloadImpl(payload);
	}

	/**
	 * \brief Plugin-specific implementation hook for serializing one frame payload.
	 *
	 * \param payload Pointer to the frame integral-payload vector.
	 * \return \c true on success, \c false on failure.
	 */
	virtual bool publishPayloadImpl([[maybe_unused]] const std::vector<GIntegralPayload*>* payload) { return false; }

	/**
	 * \brief End publishing one frame stream record.
	 *
	 * \param frameRunData Frame collection being published.
	 * \return \c true on success, \c false on failure.
	 */
	[[nodiscard]] bool endStream([[maybe_unused]] const GFrameDataCollection* frameRunData) {
		log->debug(NORMAL, "GStreamer::endStream");
		return endStreamImpl(frameRunData);
	}

	/**
	 * \brief Plugin-specific implementation hook called at the end of a frame stream record.
	 *
	 * \param frameRunData Frame collection being published.
	 * \return \c true on success, \c false on failure.
	 */
	virtual bool endStreamImpl([[maybe_unused]] const GFrameDataCollection* frameRunData) { return false; }

	/**
	 * \brief Flush all buffered events to the backend in publish order.
	 *
	 * For each buffered event, the base class executes the event publish sequence and then clears
	 * the internal buffer. This method is called automatically when buffering reaches its threshold,
	 * when the connection is closed, and before frame streaming begins.
	 */
	void flushEventBuffer();

private:
	/**
	 * \brief Return the final backend-specific output filename.
	 *
	 * Derived plugins must implement this method to append the correct extension or otherwise map
	 * the current \ref gstreamer_definitions configuration to a final backend name.
	 *
	 * \return Output filename for the current streamer instance.
	 */
	[[nodiscard]] virtual std::string filename() const = 0;

	/// \brief Buffered events waiting to be flushed to the backend.
	std::vector<std::shared_ptr<GEventDataCollection>> eventBuffer;

	/// \brief Maximum number of buffered events before automatic flush.
	size_t bufferFlushLimit = 10;

public:
	/**
	 * \brief Instantiate a streamer plugin from a dynamic library handle.
	 *
	 * The method resolves the required \c GStreamerFactory symbol and invokes it with the supplied
	 * options object.
	 *
	 * \param h Dynamic library handle.
	 * \param g Parsed options container forwarded to the plugin constructor.
	 * \return Newly created plugin instance, or \c nullptr when the handle or symbol is invalid.
	 */
	static GStreamer* instantiate(const dlhandle h, std::shared_ptr<GOptions> g) {
		if (!h) return nullptr;
		using fptr = GStreamer* (*)(std::shared_ptr<GOptions>);

		auto sym = dlsym(h, "GStreamerFactory");
		if (!sym) return nullptr;

		auto func = reinterpret_cast<fptr>(sym);
		return func(g);
	}
};


namespace gstreamer {

	using gstreamersMap = std::unordered_map<std::string, std::shared_ptr<GStreamer>>;

	/**
	 * \ingroup gstreamer_core_api
	 * \brief Create a per-thread map of configured streamer instances.
	 *
	 * This helper parses all configured output definitions, specializes each one for the requested
	 * worker thread, dynamically loads the corresponding plugin, and stores the resulting streamer
	 * object in the returned map.
	 *
	 * The helper intentionally does not open the backend connection. That step is left to the caller
	 * so applications can decide how to handle failures and when to start the output lifetime.
	 *
	 * \param gopts Parsed options container supplying streamer definitions.
	 * \param thread_id Worker thread id used to specialize output names. The default leaves names unchanged.
	 * \return Shared pointer to a constant map from plugin object name to streamer instance.
	 */
	inline std::shared_ptr<const gstreamersMap> gstreamersMapPtr(const std::shared_ptr<GOptions>& gopts,
																 int                              thread_id = -1) {
		auto log = std::make_shared<GLogger>(gopts, "gstreamersMap worker for thread id" + std::to_string(thread_id),
											 GSTREAMER_LOGGER);

		GManager manager(gopts);

		auto gstreamers = std::make_shared<gstreamersMap>();

		for (const auto& gstreamer_def : gstreamer::getGStreamerDefinition(gopts)) {
			auto        gstreamer_def_thread = GStreamerDefinition(gstreamer_def, thread_id);
			std::string gstreamer_plugin     = gstreamer_def_thread.gstreamerPluginName();

			// Load the plugin object for this configured output.
			auto streamer = manager.LoadAndRegisterObjectFromLibrary<GStreamer>(gstreamer_plugin, gopts);
			gstreamers->emplace(gstreamer_plugin, streamer);

			// Bind the thread-specialized definition to the plugin instance.
			gstreamers->at(gstreamer_plugin)->define_gstreamer(gstreamer_def_thread);
		}

		return gstreamers;
	}

} // namespace gstreamer