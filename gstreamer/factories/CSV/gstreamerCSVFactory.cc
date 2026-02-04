// gstreamer
#include "gstreamerCSVFactory.h"

// Non-Doxygen implementation file: behavior is documented in the header.

// tells the DLL how to create a GStreamerFactory in each plugin .so/.dylib
extern "C" GStreamer* GStreamerFactory(const std::shared_ptr<GOptions>& g) {
	return  static_cast<GStreamer*> (new GstreamerCsvFactory(g));
}
