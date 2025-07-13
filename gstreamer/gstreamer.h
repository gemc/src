#pragma once

// gstreamer
#include "gstreamer_options.h"

// gemc
#include "event/gEventDataCollection.h"
#include "frame/gFrameDataCollection.h"
#include "gfactory.h"
#include "glogger.h"

// c++
#include <string>

class GStreamer {

public:
	virtual ~GStreamer() = default;

	[[nodiscard]] bool openConnection() {
		log->debug(NORMAL, "GStreamer::openConnection -> ", filename());
		return openConnectionImpl();
	}

	virtual bool openConnectionImpl() { return false; }

	[[nodiscard]] bool closeConnection() {
		log->debug(NORMAL, "GStreamer::closeConnection <- ", filename());
		return closeConnectionImpl();
	}

	virtual bool closeConnectionImpl() { return false; }

	// called in GRunAction::EndOfRunAction
	// runs the protected virtual methods below to write events from a run to file
	// the key is the routine name and sensitive detector name
	std::map<std::string, bool> publishEventRunData(const std::vector<GEventDataCollection*>& runData);

	// called in GRunAction::EndOfRunAction
	// runs the protected virtual methods below to write frames from a run to file
	// the key is the routine name and frame streamer id
	std::map<std::string, bool> publishFrameRunData(const std::shared_ptr<GLogger>& log, const GFrameDataCollection* frameRunData);

	[[nodiscard]] inline std::string getStreamType() const { return gstreamer_definitions.type; }
	inline void define_gstreamer(const GStreamerDefinition& gstreamerDefinition, int tid = -1) { gstreamer_definitions = GStreamerDefinition(gstreamerDefinition, tid); }

	static const std::vector<std::string> supported_formats;
	static bool                           is_valid_format(const std::string& format);

	void set_loggers(GOptions* const g) { log = std::make_shared<GLogger>(g, GSTREAMER_LOGGER, "streamer logger"); }

protected:
	/// Data, Translation Tables, and digitization loggers.
	std::shared_ptr<GLogger> log;

	GStreamerDefinition gstreamer_definitions;

	// event virtual methods called by publishRunData, in order
	// --------------------------------------------------------

	[[nodiscard]] bool startEvent([[maybe_unused]] const GEventDataCollection* eventData) {
		log->debug(NORMAL, "GStreamer::startEvent");
		return startEventImpl(eventData);
	}

	virtual bool startEventImpl([[maybe_unused]] const GEventDataCollection* eventData) { return false; }

	[[nodiscard]] bool publishEventHeader([[maybe_unused]] const GEventDataCollectionHeader* gheader) {
		log->debug(NORMAL, "GStreamer::publishEventHeader");
		return publishEventHeaderImpl(gheader);
	}

	virtual bool publishEventHeaderImpl([[maybe_unused]] const GEventDataCollectionHeader* gheader) { return false; }

	// vector index is hit number

	[[nodiscard]] bool publishEventTrueInfoData([[maybe_unused]] const std::string&                 detectorName,
	                                            [[maybe_unused]] const std::vector<GTrueInfoData*>* trueInfoData) {
		log->debug(NORMAL, "GStreamer::publishEventTrueInfoData for detector ", detectorName);
		return publishEventTrueInfoDataImpl(detectorName, trueInfoData);
	}

	virtual bool publishEventTrueInfoDataImpl([[maybe_unused]] const std::string                  detectorName,
	                                          [[maybe_unused]] const std::vector<GTrueInfoData*>* trueInfoData) { return false; }


	[[nodiscard]] bool publishEventDigitizedData([[maybe_unused]] const std::string&                  detectorName,
	                                             [[maybe_unused]] const std::vector<GDigitizedData*>* digitizedData) {
		log->debug(NORMAL, "GStreamer::publishEventDigitizedData for detector ", detectorName);
		return publishEventDigitizedDataImpl(detectorName, digitizedData);
	}

	virtual bool publishEventDigitizedDataImpl([[maybe_unused]] const std::string                   detectorName,
	                                           [[maybe_unused]] const std::vector<GDigitizedData*>* digitizedData) { return false; }


	[[nodiscard]] bool endEvent([[maybe_unused]] const GEventDataCollection* eventData) {
		log->debug(NORMAL, "GStreamer::endEvent");
		return endEventImpl(eventData);
	}

	virtual bool endEventImpl([[maybe_unused]] const GEventDataCollection* eventData) { return false; }

	// stream virtual methods
	[[nodiscard]] bool startStream([[maybe_unused]] const GFrameDataCollection* frameRunData) {
		log->debug(NORMAL, "GStreamer::startStream");
		return startStreamImpl(frameRunData);
	}

	virtual bool startStreamImpl([[maybe_unused]] const GFrameDataCollection* frameRunData) { return false; }

	[[nodiscard]] bool publishFrameHeader([[maybe_unused]] const GFrameDataCollectionHeader* gframeHeader) {
		log->debug(NORMAL, "GStreamer::publishFrameHeader");
		return publishFrameHeaderImpl(gframeHeader);
	}

	virtual bool publishFrameHeaderImpl([[maybe_unused]] const GFrameDataCollectionHeader* gframeHeader) { return false; }

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

private:
	[[nodiscard]] virtual std::string filename() const = 0; // must be implemented in derived classes

public:
	// method to dynamically load factories
	static GStreamer* instantiate(const dlhandle handle) {
		if (handle == nullptr) return nullptr;

		// must match the extern C declaration in the derived factories
		void* maker = dlsym(handle, "GStreamerFactory");

		if (maker == nullptr) return nullptr;

		typedef GStreamer*(*fptr)();

		// static_cast not allowed here
		// see http://stackoverflow.com/questions/573294/when-to-use-reinterpret-cast
		// need to run the DLL GMediaFactory function that returns the factory
		fptr func = reinterpret_cast<fptr>(reinterpret_cast<void*>(maker));

		return func();
	}

};



namespace gstreamer {

inline std::unordered_map<std::string, std::shared_ptr<GStreamer>> gstreamersMap(const std::vector<GStreamerDefinition>& goutput_defs,
                                                                                 int                                     tid,
                                                                                 GOptions*                               gopts,
                                                                                 std::shared_ptr<GLogger>                log) {
	GManager manager(log);

	std::unordered_map<std::string, std::shared_ptr<GStreamer>> gstreamersMap;

	for (const auto& goutput_def : goutput_defs) {

		auto        goutput_def_thread = GStreamerDefinition(goutput_def, tid);
		std::string gstreamer_plugin   = goutput_def_thread.gstreamerPluginName();

		gstreamersMap.emplace(gstreamer_plugin,
		                      manager.LoadAndRegisterObjectFromLibrary<GStreamer>(gstreamer_plugin, gopts));
		gstreamersMap[gstreamer_plugin]->define_gstreamer(goutput_def_thread);
		if (!gstreamersMap[gstreamer_plugin]->openConnection()) {
			log->error(1, "Failed to open connection for GStreamer ", gstreamer_plugin, " in thread ", tid);
		}
	}

	// Freeze the map before passing it to worker threads
	// unordered_map is read-only the entire time the event threads run, and the C ++standard
	// guarantees that concurrent reads on a const container are safe so long as no thread mutates it
	const auto& gstreamerConstMap = gstreamersMap; // const reference

	return gstreamerConstMap;
}

// returns a vector of identical gstreamersMap the size of the number of threads
inline std::vector<std::unordered_map<std::string, std::shared_ptr<GStreamer>>> gstreamersMapVector(const std::vector<GStreamerDefinition>& goutput_defs,
																										   int                                     nthreads,
																										   GOptions*                               gopts,
																										   std::shared_ptr<GLogger>                log) {
	std::vector<std::unordered_map<std::string, std::shared_ptr<GStreamer>>> gstreamersMaps;

	for (int tid = 0; tid < nthreads; ++tid) {
		gstreamersMaps.emplace_back(gstreamersMap(goutput_defs, tid, gopts, log));
	}

	return gstreamersMaps;
}

} // namespace gstreamer
