#pragma once

// gemc
#include "event/gEventDataCollection.h"
#include "frame/gFrameDataCollection.h"
#include "gfactory.h"
#include "glogger.h"

// c++
#include <string>
#include <utility>

// utility struct to load GStreamer from options
struct GStreamerDefinition {

	// default constructor
	GStreamerDefinition() = default;

	GStreamerDefinition(std::string f, std::string n, std::string t) : format(std::move(f)), name(std::move(n)), type(std::move(t)) {
	}

	std::string format;
	std::string name;
	std::string type;

	[[nodiscard]] std::string gstreamerPluginName() const { return "gstreamer" + format + "Factory"; }
};

class GStreamer {

public:
	virtual ~GStreamer() = default;

	virtual bool openConnection([[maybe_unused]] const std::shared_ptr<GLogger>& log) { return false; }  // in GActionInitialization constructor
	virtual bool closeConnection([[maybe_unused]] const std::shared_ptr<GLogger>& log) { return false; } // in GActionInitialization destructor

	// called in GRunAction::EndOfRunAction
	// runs the protected virtual methods below to write events from a run to file
	// the key is the routine name and sensitive detector name
	std::map<std::string, bool> publishEventRunData(std::shared_ptr<GLogger>& log, const std::vector<GEventDataCollection*>& runData);

	// called in GRunAction::EndOfRunAction
	// runs the protected virtual methods below to write frames from a run to file
	// the key is the routine name and frame streamer id
	std::map<std::string, bool> publishFrameRunData(const std::shared_ptr<GLogger>& log, const GFrameDataCollection* frameRunData);

	[[nodiscard]] inline std::string getStreamType() const { return gstreamer_definitions.type; }
	inline void                 define_gstreamer(GStreamerDefinition gstreamerDefinition) { gstreamer_definitions = std::move(gstreamerDefinition); }

	static const std::vector<std::string> supported_formats;
	static bool                 is_valid_format(const std::string& format);

protected:
	GStreamerDefinition gstreamer_definitions;

	// event virtual methods called by publishRunData, in order
	// --------------------------------------------------------

	virtual bool startEvent([[maybe_unused]] const GEventDataCollection*     eventData,
	                        [[maybe_unused]] const std::shared_ptr<GLogger>& log) { return false; }

	virtual bool publishEventHeader([[maybe_unused]] const GEventDataCollectionHeader* gheader,
	                                [[maybe_unused]] std::shared_ptr<GLogger>&   log) { return false; }

	// vector index is hit number
	virtual bool
	publishEventTrueInfoData([[maybe_unused]] const std::string                  detectorName,
	                         [[maybe_unused]] const std::vector<GTrueInfoData*>* trueInfoData,
	                         [[maybe_unused]] std::shared_ptr<GLogger>&     log) { return false; }

	virtual bool
	publishEventDigitizedData([[maybe_unused]] const std::string                   detectorName,
	                          [[maybe_unused]] const std::vector<GDigitizedData*>* digitizedData,
	                          [[maybe_unused]] std::shared_ptr<GLogger>&      log) { return false; }

	virtual bool endEvent([[maybe_unused]] const GEventDataCollection*     eventData,
	                      [[maybe_unused]] const std::shared_ptr<GLogger>& log) { return false; }

	// stream virtual methods
	virtual bool startStream([[maybe_unused]] const GFrameDataCollection*     frameRunData,
	                         [[maybe_unused]] const std::shared_ptr<GLogger>& log) { return false; }

	virtual bool publishFrameHeader([[maybe_unused]] const GFrameDataCollectionHeader* gframeHeader,
	                                [[maybe_unused]] const std::shared_ptr<GLogger>&   log) { return false; }

	virtual bool publishPayload([[maybe_unused]] const std::vector<GIntegralPayload*>* payload,
	                            [[maybe_unused]] const std::shared_ptr<GLogger>&  log) { return false; }

	virtual bool endStream([[maybe_unused]] const GFrameDataCollection*     frameRunData,
	                       [[maybe_unused]] const std::shared_ptr<GLogger>& log) { return false; }

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
