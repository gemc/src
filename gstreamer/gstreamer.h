#pragma once

// gstreamer
#include "gstreamer_options.h"
#include "gstreamerConventions.h"

// gemc
#include "event/gEventDataCollection.h"
#include "frame/gFrameDataCollection.h"
#include "gfactory.h"
#include "glogger.h"

// c++
#include <string>
#include <vector>
#include <map>


class GStreamer {

public:
	// The destructor must be virtual to ensure that
	// the derived constructors are called — otherwise only the base
	// class destructor runs, leading to resource leaks or undefined behavior.
	virtual ~GStreamer() = default;

	[[nodiscard]] virtual bool openConnection() { return false; }

	[[nodiscard]] bool closeConnection() {
		flushEventBuffer();
		return closeConnectionImpl();
	}

	[[nodiscard]] virtual bool closeConnectionImpl() { return false; }

	// runs the protected virtual methods below to write events from a run to file
	void publishEventData(const std::shared_ptr<GEventDataCollection>& event_data);

	// runs the protected virtual methods below to write frames from a run to file
	void publishFrameRunData(const std::shared_ptr<GFrameDataCollection>& frameRunData);

	[[nodiscard]] inline std::string getStreamType() const { return gstreamer_definitions.type; }
	inline void define_gstreamer(const GStreamerDefinition& gstreamerDefinition, int tid = -1) { gstreamer_definitions = GStreamerDefinition(gstreamerDefinition, tid); }

	static const std::vector<std::string> supported_formats;
	static bool                           is_valid_format(const std::string& format);

	void set_loggers(const std::shared_ptr<GOptions>& g) {
		bufferFlushLimit = g->getScalarInt("ebuffer");
		log              = std::make_shared<GLogger>(g, GSTREAMER_LOGGER, "streamer logger");
	}

protected:
	/// Data, Translation Tables, and digitization loggers.
	std::shared_ptr<GLogger> log;

	GStreamerDefinition gstreamer_definitions;

	// event virtual methods called by publishRunData, in order
	// notice here we pass const raw pointers to the event data

	[[nodiscard]] bool startEvent([[maybe_unused]] const std::shared_ptr<GEventDataCollection>& event_data) {
		if (!event_data) { log->error(ERR_PUBLISH_ERROR, "eventData is null in GStreamer::startEvent"); }
		if (!event_data->getHeader()) { log->error(ERR_PUBLISH_ERROR, "event header is null in GStreamer::startEvent"); }

		log->debug(NORMAL, "GStreamer::startEvent");
		return startEventImpl(event_data);
	}

	virtual bool startEventImpl([[maybe_unused]] const std::shared_ptr<GEventDataCollection>& event_data) { return false; }

	[[nodiscard]] bool publishEventHeader([[maybe_unused]] const std::unique_ptr<GEventHeader>& gheader) {
		if (!gheader) { log->error(ERR_PUBLISH_ERROR, "event header is null in GStreamer::publishEventHeader"); }
		log->debug(NORMAL, "GStreamer::publishEventHeader");
		return publishEventHeaderImpl(gheader);
	}

	virtual bool publishEventHeaderImpl([[maybe_unused]] const std::unique_ptr<GEventHeader>& gheader) { return false; }

	// vector index is hit number
	[[nodiscard]] bool publishEventTrueInfoData([[maybe_unused]] const std::string&                       detectorName,
	                                            [[maybe_unused]] const std::vector<const GTrueInfoData*>& trueInfoData) {
		log->debug(NORMAL, "GStreamer::publishEventTrueInfoData for detector ", detectorName);
		return publishEventTrueInfoDataImpl(detectorName, trueInfoData);
	}

	virtual bool publishEventTrueInfoDataImpl([[maybe_unused]] const std::string&                       detectorName,
	                                          [[maybe_unused]] const std::vector<const GTrueInfoData*>& trueInfoData) { return false; }


	[[nodiscard]] bool publishEventDigitizedData([[maybe_unused]] const std::string&                        detectorName,
	                                             [[maybe_unused]] const std::vector<const GDigitizedData*>& digitizedData) {
		log->debug(NORMAL, "GStreamer::publishEventDigitizedData for detector ", detectorName);
		return publishEventDigitizedDataImpl(detectorName, digitizedData);
	}

	virtual bool publishEventDigitizedDataImpl([[maybe_unused]] const std::string&                        detectorName,
	                                           [[maybe_unused]] const std::vector<const GDigitizedData*>& digitizedData) { return false; }


	[[nodiscard]] bool endEvent([[maybe_unused]] const std::shared_ptr<GEventDataCollection>& event_data) {
		log->debug(NORMAL, "GStreamer::endEvent");
		return endEventImpl(event_data);
	}

	virtual bool endEventImpl([[maybe_unused]] const std::shared_ptr<GEventDataCollection>& event_data) { return false; }

	// stream virtual methods
	[[nodiscard]] bool startStream([[maybe_unused]] const GFrameDataCollection* frameRunData) {
		flushEventBuffer();
		log->debug(NORMAL, "GStreamer::startStream");
		return startStreamImpl(frameRunData);
	}

	virtual bool startStreamImpl([[maybe_unused]] const GFrameDataCollection* frameRunData) { return false; }

	[[nodiscard]] bool publishFrameHeader([[maybe_unused]] const GFrameHeader* gframeHeader) {
		log->debug(NORMAL, "GStreamer::publishFrameHeader");
		return publishFrameHeaderImpl(gframeHeader);
	}

	virtual bool publishFrameHeaderImpl([[maybe_unused]] const GFrameHeader* gframeHeader) { return false; }

	[[nodiscard]] bool publishPayload([[maybe_unused]] const std::vector<GIntegralPayload*>* payload) {
		log->debug(NORMAL, "GStreamer::publishPayload");
		return publishPayloadImpl(payload);
	}

	virtual bool publishPayloadImpl([[maybe_unused]] const std::vector<GIntegralPayload*>* payload) { return false; }

	[[nodiscard]] bool endStream([[maybe_unused]] const GFrameDataCollection* frameRunData) {
		log->debug(NORMAL, "GStreamer::endStream");
		return endStreamImpl(frameRunData);
	}

	virtual bool endStreamImpl([[maybe_unused]] const GFrameDataCollection* frameRunData) { return false; }

	// flush the event buffer, writing all events to file
	void flushEventBuffer();

private:
	[[nodiscard]] virtual std::string filename() const = 0; // must be implemented in derived classes

	// storing raw pointers here
	std::vector<std::shared_ptr<GEventDataCollection>> eventBuffer;
	size_t                                             bufferFlushLimit = 10; // default; can be overridden

public:
	// method to dynamically load factories
	// static GStreamer* instantiate(const dlhandle handle) {
	// 	if (handle == nullptr) return nullptr;
	//
	// 	// must match the extern C declaration in the derived factories
	// 	void* maker = dlsym(handle, "GStreamerFactory");
	//
	// 	if (maker == nullptr) return nullptr;
	//
	// 	typedef GStreamer*(*fptr)();
	//
	// 	// static_cast not allowed here
	// 	// see http://stackoverflow.com/questions/573294/when-to-use-reinterpret-cast
	// 	// need to run the DLL GMediaFactory function that returns the factory
	// 	fptr func = reinterpret_cast<fptr>(reinterpret_cast<void*>(maker));
	//
	// 	return func();
	// }

	static GStreamer* instantiate(const dlhandle h, std::shared_ptr<GOptions> g) {
		if (!h) return nullptr;
		using fptr = GStreamer* (*)(std::shared_ptr<GOptions>);

		// Must match the extern "C" declaration in the derived factories.
		auto sym   = dlsym(h, "GStreamerFactory");
		if (!sym) return nullptr;

		auto func = reinterpret_cast<fptr>(sym);
		return func(g);
	}

};


namespace gstreamer {

using gstreamersMap = std::unordered_map<std::string, std::shared_ptr<GStreamer>>;

// this run in a worker thread, so each thread gets its own map of gstreamers
inline std::shared_ptr<const gstreamersMap> gstreamersMapPtr(const std::shared_ptr<GOptions>& gopts,
                                                             int                              thread_id) {
	auto log = std::make_shared<GLogger>(gopts, GSTREAMER_LOGGER, "gstreamersMap worker for thread id" + std::to_string(thread_id));

	GManager manager(gopts);

	auto gstreamers = std::make_shared<gstreamersMap>();

	for (const auto& gstreamer_def : gstreamer::getGStreamerDefinition(gopts)) {
		auto        gstreamer_def_thread = GStreamerDefinition(gstreamer_def, thread_id);
		std::string gstreamer_plugin     = gstreamer_def_thread.gstreamerPluginName();

		auto gstr_ptr =  manager.LoadAndRegisterObjectFromLibrary<GStreamer>(gstreamer_plugin, gopts);

		auto streamer = manager.LoadAndRegisterObjectFromLibrary<GStreamer>(gstreamer_plugin, gopts);
		gstreamers->emplace(gstreamer_plugin, streamer);

		gstreamers->at(gstreamer_plugin)->define_gstreamer(gstreamer_def_thread);

		// TODO: maybe do not do this here, should be done by API
		if (!gstreamers->at(gstreamer_plugin)->openConnection()) {
			log->error(1, "Failed to open connection for GStreamer ", gstreamer_plugin, " in thread ", gstreamer_def_thread.tid);
		}
	}

	return gstreamers;
}


} // namespace gstreamer
