#pragma once

// gemc
#include "goptions.h"

// c++
#include <vector>

constexpr const char* GSTREAMER_LOGGER = "gstreamer";

// utility struct to load GStreamer from options
struct GStreamerDefinition {

	// default constructor
	GStreamerDefinition() = default;

	GStreamerDefinition(std::string f, std::string n, std::string t) :
		format(std::move(f)), name(std::move(n)), type(std::move(t)) {
	}

	std::string format;
	std::string name;
	std::string type;

	[[nodiscard]] std::string gstreamerPluginName() const { return "gstreamer_" + format + "_plugin"; }
};


namespace gstreamer {

// method to return a vector of GStreamerDefinition from the options
std::vector<GStreamerDefinition> getGStreamerDefinition(GOptions* gopts);

// returns array of options definitions
GOptions defineOptions();
}
