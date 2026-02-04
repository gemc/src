#pragma once

// gstreamer
#include "gstreamer_options.h"
#include "gstreamerConventions.h"

// gemc
#include "event/gEventDataCollection.h"
#include "frame/gFrameDataCollection.h"
#include "gfactory.h"
#include "gbase.h"

// c++
#include <string>
#include <vector>
#include <map>

/**
 * \file gstreamer.h
 * \brief Core streaming interface for gstreamer output plugins.
 */

/**
 * \class GStreamer
 * \brief Abstract base class for streaming GEMC event or frame data to output media.
 *
 * \section gstreamer_lifecycle Lifecycle
 * A typical usage sequence for a concrete streamer is:
 * 1. Construct a derived streamer (usually through plugin loading).
 * 2. Configure it by calling \ref define_gstreamer "define_gstreamer()"
 *    and \ref set_loggers "set_loggers()".
 * 3. Open the underlying output medium via \ref openConnection "openConnection()".
 * 4. Publish data:
 *    - Events: call \ref publishEventData "publishEventData()" for each event.
 *      Events are buffered and written out when the buffer reaches a configured limit,
 *      or when \ref closeConnection "closeConnection()" is invoked.
 *    - Frames: frame streaming hooks exist, but frame publishing is currently plugin-defined
 *      (the base class provides the protected hook sequence).
 * 5. Close the output medium via \ref closeConnection "closeConnection()".
 *
 * \section gstreamer_buffering Buffering model
 * The base class stores a per-streamer in-memory buffer of \c std::shared_ptr<GEventDataCollection>.
 * The buffer is flushed by \ref flushEventBuffer "flushEventBuffer()" when:
 * - the number of buffered events reaches \c bufferFlushLimit, or
 * - \ref closeConnection "closeConnection()" is called, or
 * - \ref startStream "startStream()" is called (to avoid mixing event and frame streams).
 *
 * During flushing, each event is treated as read-only: the base class extracts raw pointers
 * from hit containers and passes those raw pointers to plugin hooks. The raw pointers remain
 * valid for the duration of the flush because the owning event shared pointer is kept alive
 * by the buffer.
 *
 * \section gstreamer_threading Threading expectations
 * The gstreamer module is typically used with one streamer instance per worker thread.
 * The helper gstreamer::gstreamersMapPtr() creates such per-thread streamer maps.
 * The base class itself does not provide external synchronization; therefore:
 * - Do not share a single streamer instance across multiple threads unless the derived class
 *   implements its own locking.
 *
 * \section gstreamer_plugins Plugin factory symbol
 * Streamer plugins are loaded through a dynamic loader. Each plugin must expose an extern "C"
 * function named \c GStreamerFactory that returns a new \ref GStreamer instance. The helper
 * \ref instantiate "instantiate()" resolves that symbol via \c dlsym.
 */
class GStreamer : public GBase<GStreamer>
{
public:
	/**
	 * \brief Construct a streamer and bind it to module logging.
	 * \param g Options container used to initialize logging and configuration.
	 */
	explicit GStreamer(const std::shared_ptr<GOptions>& g) : GBase(g, GSTREAMER_LOGGER) {
	}

	/**
	 * \brief Virtual destructor.
	 *
	 * The destructor is virtual to ensure derived destructors run correctly when deleting
	 * through a base pointer.
	 */
	virtual ~GStreamer() = default;

	/**
	 * \brief Open the output medium (file, socket, etc.).
	 *
	 * Derived classes override this to acquire resources and validate accessibility.
	 * \return \c true on success, \c false on failure.
	 */
	[[nodiscard]] virtual bool openConnection() { return false; }

	/**
	 * \brief Close the output medium after flushing any buffered events.
	 *
	 * This calls \ref flushEventBuffer "flushEventBuffer()" first, then delegates to
	 * \ref closeConnectionImpl "closeConnectionImpl()".
	 *
	 * \return \c true on success, \c false on failure.
	 */
	[[nodiscard]] bool closeConnection() {
		flushEventBuffer();
		return closeConnectionImpl();
	}

	/**
	 * \brief Implementation hook for closing the output medium.
	 *
	 * Derived classes override this to release resources (close files, detach trees, etc.).
	 * \return \c true on success, \c false on failure.
	 */
	[[nodiscard]] virtual bool closeConnectionImpl() { return false; }

	/**
	 * \brief Buffer an event for later serialization.
	 *
	 * The event is appended to an internal buffer. When the buffer reaches the configured
	 * limit (\c bufferFlushLimit), the streamer flushes all buffered events in a single pass.
	 *
	 * \param event_data Event container to publish.
	 */
	void publishEventData(const std::shared_ptr<GEventDataCollection>& event_data);

	// runs the protected virtual methods below to write frames from a run to file
	// void publishFrameRunData(const std::shared_ptr<GFrameDataCollection>& frameRunData);

	/**
	 * \brief Return the semantic stream type for this streamer.
	 *
	 * This value comes from \ref gstreamer_definitions and is typically configured via options.
	 * \return The type string (e.g. \c "event" or \c "stream").
	 */
	[[nodiscard]] inline std::string getStreamType() const { return gstreamer_definitions.type; }

	/**
	 * \brief Assign the output definition used by this streamer instance.
	 *
	 * The caller may specify a thread id to specialize the filename. A negative thread id keeps
	 * the original base filename unchanged.
	 *
	 * \param gstreamerDefinition Output definition (format, base filename, type).
	 * \param tid Thread id used to specialize the filename (defaults to -1, meaning "no specialization").
	 */
	inline void define_gstreamer(const GStreamerDefinition& gstreamerDefinition, int tid = -1) {
		gstreamer_definitions = GStreamerDefinition(gstreamerDefinition, tid);
	}

	/**
	 * \brief Return the list of supported output formats.
	 *
	 * This is a function-local static instead of a global variable to avoid static destruction order issues.
	 * \return Reference to a stable list of supported format tokens.
	 */
	static const std::vector<std::string>& supported_formats();

	/**
	 * \brief Check whether a format token is supported.
	 * \param format Format token to validate (case-insensitive).
	 * \return \c true if the format is supported, \c false otherwise.
	 */
	static bool is_valid_format(const std::string& format);

	/**
	 * \brief Configure streamer settings derived from options.
	 *
	 * Currently this extracts \c ebuffer and sets \c bufferFlushLimit.
	 * \param g Options container.
	 */
	void set_loggers(const std::shared_ptr<GOptions>& g) {
		bufferFlushLimit = g->getScalarInt("ebuffer");
	}

protected:
	/// \brief Output definition used by this streamer (format, base name, type, thread id).
	GStreamerDefinition gstreamer_definitions;

	// Event virtual methods called during buffer flushing, in order.
	// Each hook returns a bool for "success/failure" to support uniform logging and diagnostics.

	/**
	 * \brief Begin an event publish sequence.
	 *
	 * This wrapper validates \p event_data and its header, logs a debug trace, then calls
	 * \ref startEventImpl "startEventImpl()".
	 *
	 * \param event_data Event being published.
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
	 * \brief Implementation hook for beginning an event publish sequence.
	 * \param event_data Event being published.
	 * \return \c true on success, \c false on failure.
	 */
	virtual bool startEventImpl([[maybe_unused]] const std::shared_ptr<GEventDataCollection>& event_data) {
		return false;
	}

	/**
	 * \brief Publish the event header.
	 *
	 * This wrapper validates \p gevent_header, logs a debug trace, then calls
	 * \ref publishEventHeaderImpl "publishEventHeaderImpl()".
	 *
	 * \param gevent_header Event header unique pointer owned by the event container.
	 * \return \c true on success, \c false on failure.
	 */
	[[nodiscard]] bool publishEventHeader([[maybe_unused]] const std::unique_ptr<GEventHeader>& gevent_header) {
		if (!gevent_header) { log->error(ERR_PUBLISH_ERROR, "event header is null in GStreamer::publishEventHeader"); }
		log->debug(NORMAL, "GStreamer::publishEventHeader");
		return publishEventHeaderImpl(gevent_header);
	}

	/**
	 * \brief Implementation hook for publishing the event header.
	 * \param gevent_header Event header unique pointer owned by the event container.
	 * \return \c true on success, \c false on failure.
	 */
	virtual bool publishEventHeaderImpl([[maybe_unused]] const std::unique_ptr<GEventHeader>& gevent_header) {
		return false;
	}

	/**
	 * \brief Publish true (MC) information hits for one detector.
	 *
	 * The \p trueInfoData vector contains raw pointers that remain valid during the flush because
	 * the owning hit containers are owned by the buffered event.
	 *
	 * \param detectorName Detector / sensitive detector name (map key in the event collection).
	 * \param trueInfoData Raw pointers to true info hits. The vector index corresponds to hit index.
	 * \return \c true on success, \c false on failure.
	 */
	[[nodiscard]] bool publishEventTrueInfoData([[maybe_unused]] const std::string& detectorName,
	                                            [[maybe_unused]] const std::vector<const GTrueInfoData*>&
	                                            trueInfoData) {
		log->debug(NORMAL, "GStreamer::publishEventTrueInfoData for detector ", detectorName);
		return publishEventTrueInfoDataImpl(detectorName, trueInfoData);
	}

	/**
	 * \brief Implementation hook for publishing true info hits for one detector.
	 * \param detectorName Detector / sensitive detector name.
	 * \param trueInfoData Raw pointers to true info hits.
	 * \return \c true on success, \c false on failure.
	 */
	virtual bool publishEventTrueInfoDataImpl([[maybe_unused]] const std::string&                       detectorName,
	                                          [[maybe_unused]] const std::vector<const GTrueInfoData*>& trueInfoData) {
		return false;
	}


	/**
	 * \brief Publish digitized hits for one detector.
	 *
	 * The \p digitizedData vector contains raw pointers that remain valid during the flush because
	 * the owning hit containers are owned by the buffered event.
	 *
	 * \param detectorName Detector / sensitive detector name (map key in the event collection).
	 * \param digitizedData Raw pointers to digitized hits. The vector index corresponds to hit index.
	 * \return \c true on success, \c false on failure.
	 */
	[[nodiscard]] bool publishEventDigitizedData([[maybe_unused]] const std::string& detectorName,
	                                             [[maybe_unused]] const std::vector<const GDigitizedData*>&
	                                             digitizedData) {
		log->debug(NORMAL, "GStreamer::publishEventDigitizedData for detector ", detectorName);
		return publishEventDigitizedDataImpl(detectorName, digitizedData);
	}

	/**
	 * \brief Implementation hook for publishing digitized hits for one detector.
	 * \param detectorName Detector / sensitive detector name.
	 * \param digitizedData Raw pointers to digitized hits.
	 * \return \c true on success, \c false on failure.
	 */
	virtual bool publishEventDigitizedDataImpl([[maybe_unused]] const std::string& detectorName,
	                                           [[maybe_unused]] const std::vector<const GDigitizedData*>&
	                                           digitizedData) { return false; }


	/**
	 * \brief End an event publish sequence.
	 * \param event_data Event being published.
	 * \return \c true on success, \c false on failure.
	 */
	[[nodiscard]] bool endEvent([[maybe_unused]] const std::shared_ptr<GEventDataCollection>& event_data) {
		log->debug(NORMAL, "GStreamer::endEvent");
		return endEventImpl(event_data);
	}

	/**
	 * \brief Implementation hook for ending an event publish sequence.
	 * \param event_data Event being published.
	 * \return \c true on success, \c false on failure.
	 */
	virtual bool endEventImpl([[maybe_unused]] const std::shared_ptr<GEventDataCollection>& event_data) {
		return false;
	}

	// Frame stream virtual methods.
	// These hooks are provided for plugins that serialize frame-based data.
	// The base class flushes event buffers before starting a frame stream.

	/**
	 * \brief Begin a frame stream publish sequence.
	 *
	 * This wrapper flushes pending events (to avoid mixing event and frame streams),
	 * logs a debug trace, then calls \ref startStreamImpl "startStreamImpl()".
	 *
	 * \param frameRunData Frame container being published.
	 * \return \c true on success, \c false on failure.
	 */
	[[nodiscard]] bool startStream([[maybe_unused]] const GFrameDataCollection* frameRunData) {
		flushEventBuffer();
		log->debug(NORMAL, "GStreamer::startStream");
		return startStreamImpl(frameRunData);
	}

	/**
	 * \brief Implementation hook for beginning a frame stream publish sequence.
	 * \param frameRunData Frame container being published.
	 * \return \c true on success, \c false on failure.
	 */
	virtual bool startStreamImpl([[maybe_unused]] const GFrameDataCollection* frameRunData) { return false; }

	/**
	 * \brief Publish a frame header.
	 * \param gframeHeader Frame header pointer.
	 * \return \c true on success, \c false on failure.
	 */
	[[nodiscard]] bool publishFrameHeader([[maybe_unused]] const GFrameHeader* gframeHeader) {
		log->debug(NORMAL, "GStreamer::publishFrameHeader");
		return publishFrameHeaderImpl(gframeHeader);
	}

	/**
	 * \brief Implementation hook for publishing a frame header.
	 * \param gframeHeader Frame header pointer.
	 * \return \c true on success, \c false on failure.
	 */
	virtual bool publishFrameHeaderImpl([[maybe_unused]] const GFrameHeader* gframeHeader) { return false; }

	/**
	 * \brief Publish a frame payload.
	 * \param payload Pointer to payload vector.
	 * \return \c true on success, \c false on failure.
	 */
	[[nodiscard]] bool publishPayload([[maybe_unused]] const std::vector<GIntegralPayload*>* payload) {
		log->debug(NORMAL, "GStreamer::publishPayload");
		return publishPayloadImpl(payload);
	}

	/**
	 * \brief Implementation hook for publishing a frame payload.
	 * \param payload Pointer to payload vector.
	 * \return \c true on success, \c false on failure.
	 */
	virtual bool publishPayloadImpl([[maybe_unused]] const std::vector<GIntegralPayload*>* payload) { return false; }

	/**
	 * \brief End a frame stream publish sequence.
	 * \param frameRunData Frame container being published.
	 * \return \c true on success, \c false on failure.
	 */
	[[nodiscard]] bool endStream([[maybe_unused]] const GFrameDataCollection* frameRunData) {
		log->debug(NORMAL, "GStreamer::endStream");
		return endStreamImpl(frameRunData);
	}

	/**
	 * \brief Implementation hook for ending a frame stream publish sequence.
	 * \param frameRunData Frame container being published.
	 * \return \c true on success, \c false on failure.
	 */
	virtual bool endStreamImpl([[maybe_unused]] const GFrameDataCollection* frameRunData) { return false; }

	/**
	 * \brief Flush the internal event buffer, writing all buffered events to the output medium.
	 *
	 * The flush sequence is:
	 * - \ref startEvent "startEvent()"
	 * - \ref publishEventHeader "publishEventHeader()"
	 * - \ref publishEventTrueInfoData "publishEventTrueInfoData()" for each detector collection
	 * - \ref publishEventDigitizedData "publishEventDigitizedData()" for each detector collection
	 * - \ref endEvent "endEvent()"
	 *
	 * After flushing, the buffer is cleared.
	 */
	void flushEventBuffer();

private:
	/**
	 * \brief Return the final output filename for this streamer instance.
	 *
	 * This must be implemented by derived classes and typically uses \ref gstreamer_definitions.rootname
	 * plus a format-specific extension.
	 *
	 * \return Output filename (including extension).
	 */
	[[nodiscard]] virtual std::string filename() const = 0; // must be implemented in derived classes

	/// \brief Buffered events waiting to be flushed. The shared pointers keep event-owned data alive.
	std::vector<std::shared_ptr<GEventDataCollection>> eventBuffer;

	/// \brief Buffer flush threshold. Default is conservative and may be overridden via options.
	size_t bufferFlushLimit = 10; // default; can be overridden

public:
	/**
	 * \brief Instantiate a streamer plugin by resolving the \c GStreamerFactory symbol from a dynamic library.
	 *
	 * \param h Dynamic library handle.
	 * \param g Options container passed to the plugin constructor.
	 * \return A new streamer instance, or \c nullptr if the handle or symbol is invalid.
	 */
	static GStreamer* instantiate(const dlhandle h, std::shared_ptr<GOptions> g) {
		if (!h) return nullptr;
		using fptr = GStreamer* (*)(std::shared_ptr<GOptions>);

		// Must match the extern "C" declaration in the derived factories.
		auto sym = dlsym(h, "GStreamerFactory");
		if (!sym) return nullptr;

		auto func = reinterpret_cast<fptr>(sym);
		return func(g);
	}
};


namespace gstreamer {
using gstreamersMap = std::unordered_map<std::string, std::shared_ptr<GStreamer>>;

/**
 * \brief Create a per-thread map of streamer instances based on configured outputs.
 *
 * This helper is intended to run inside a worker thread. It:
 * - Parses the configured gstreamer outputs from options.
 * - Specializes each output definition with \p thread_id (appends \c "_t<id>" to the base filename).
 * - Dynamically loads the corresponding plugin and registers the object.
 * - Stores each streamer in the returned map and configures it with \ref GStreamer::define_gstreamer "define_gstreamer()".
 *
 * Note:
 * - The returned map is owned by the caller via a shared pointer.
 * - Opening connections is intentionally left to the caller (to keep API flexibility).
 *
 * \param gopts Options container.
 * \param thread_id Worker thread id used to specialize output names.
 * \return Shared pointer to a map from plugin name to streamer instance.
 */
inline std::shared_ptr<const gstreamersMap> gstreamersMapPtr(const std::shared_ptr<GOptions>& gopts,
                                                             int                              thread_id) {
	auto log = std::make_shared<GLogger>(gopts, "gstreamersMap worker for thread id" + std::to_string(thread_id),
	                                     GSTREAMER_LOGGER);

	GManager manager(gopts);

	auto gstreamers = std::make_shared<gstreamersMap>();

	for (const auto& gstreamer_def : gstreamer::getGStreamerDefinition(gopts)) {
		auto        gstreamer_def_thread = GStreamerDefinition(gstreamer_def, thread_id);
		std::string gstreamer_plugin     = gstreamer_def_thread.gstreamerPluginName();

		// Load and register the streamer plugin. The loader returns a shared_ptr<GStreamer>.
		auto streamer = manager.LoadAndRegisterObjectFromLibrary<GStreamer>(gstreamer_plugin, gopts);
		gstreamers->emplace(gstreamer_plugin, streamer);

		// Bind the per-thread definition (in particular the per-thread filename) to the streamer instance.
		gstreamers->at(gstreamer_plugin)->define_gstreamer(gstreamer_def_thread);

		// Connection opening is intentionally not performed here. This is typically done by the caller
		// to control error handling and output lifetime explicitly.
		// Example:
		// if (!gstreamers->at(gstreamer_plugin)->openConnection()) {
		// 	log->error(1, "Failed to open connection for GStreamer ", gstreamer_plugin, " in thread ", gstreamer_def_thread.tid);
		// }
	}

	return gstreamers;
}
} // namespace gstreamer
