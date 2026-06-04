// gstreamer
#include "gstreamerJSONFactory.h"

// Implementation summary:
// Export the factory symbol required by the plugin loader.

extern "C" GStreamer* GStreamerFactory(const std::shared_ptr<GOptions>& g) {
	return static_cast<GStreamer*>(new GstreamerJsonFactory(g));
}