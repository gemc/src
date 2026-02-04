// gstreamer
#include "gstreamerJSONFactory.h"

// Non-Doxygen implementation notes:
// - Header file contains the authoritative API documentation.
// - This .cc focuses on behavior and correctness without duplicating the full docs.

extern "C" GStreamer* GStreamerFactory(const std::shared_ptr<GOptions>& g) {
	return static_cast<GStreamer*>(new GstreamerJsonFactory(g));
}
