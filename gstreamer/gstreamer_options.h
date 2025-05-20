#pragma once

// gstreamer
#include "gstreamer.h"

// gemc
#include "glogger.h"

constexpr const char* GSTREAMER_LOGGER = "gstreamer";

namespace gstreamer {

    // method to return a vector of GStreamerDefinition from the options
    vector<GStreamerDefinition> getGStreamerDefinition(GOptions *gopts);

    // returns array of options definitions
    GOptions defineOptions();
}

