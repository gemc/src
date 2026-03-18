// gstreamer
#include "gstreamer_options.h"
#include "gstreamerConventions.h"

#include "gdynamicdigitization_options.h"
#include "gstreamer.h"

// namespace to define options
namespace gstreamer {
using std::string;
using std::vector;

// Implementation summary:
// Parse the user-provided "gstreamer" option node into normalized
// GStreamerDefinition objects used later by plugin-loading helpers.
vector<GStreamerDefinition> getGStreamerDefinition(const std::shared_ptr<GOptions>& gopts) {
	vector<GStreamerDefinition> goutputs;

	// The "gstreamer" option node contains a list of output objects.
	// Each object is flattened into one GStreamerDefinition:
	//   - format   : plugin selector
	//   - filename : output base name
	//   - type     : semantic stream type, defaulting to "event"
	auto goutput_node = gopts->getOptionNode("gstreamer");

	for (auto goutput_item : goutput_node) {
		goutputs.emplace_back(
			gopts->get_variable_in_option<string>(goutput_item, "format", goptions::NODFLT),
			gopts->get_variable_in_option<string>(goutput_item, "filename", goptions::NODFLT),
			gopts->get_variable_in_option<string>(goutput_item, "type", "event")
		);
	}

	return goutputs;
}


// Implementation summary:
// Define the gstreamer module options and aggregate the options contributed
// by dependent modules used together with the streaming layer.
GOptions defineOptions() {
	GOptions goptions(GSTREAMER_LOGGER);

	// User-facing CLI help for the -gstreamer structured option.
	// The file extension is added automatically by the selected plugin.
	string help = "Define a Output format and name\n";
	help        += "This option defines a GStreamer output. It can be used to write events or frame streams.\n";
	help        += "\n";
	help        += "The file extension is added automatically based on the format.\n";
	help        += "Supported formats:\n";
	for (auto& format : GStreamer::supported_formats()) { help += " - " + format + "\n"; }
	help += "\n";
	help += "Output types\n";
	help += "\n";
	help += " - event: write events\n";
	help += " - stream: write frame stream\n";
	help += "\n";
	help += "Example that defines two gstreamer outputs:\n";
	help += "-gstreamer=\"[{format: root, filename: out}, {format: jlabsro, filename: out}}]\"\n";

	// Buffer flush limit:
	// controls how many events each streamer instance may retain in memory
	// before the base class forces a flush to the backend.
	goptions.defineOption(GVariable("ebuffer", DEFAULT_GSTREAMER_BUFFER_FLUSH_LIMIT,
	                                "number of events kept in memory before flushing them to the filestream"), help);

	// Schema of each object in the -gstreamer array.
	vector<GVariable> gstreamer = {
		{"filename", goptions::NODFLT, "name of output file"},
		{"format", goptions::NODFLT, "format of output file"},
		{"type", "event", "type of output file"},
	};

	goptions.defineOption(GSTREAMER_LOGGER, "define a gstreamer output", gstreamer, help);

	// Merge dependent-module options so callers can build a single composite
	// GOptions definition for applications and examples.
	goptions += gdynamicdigitization::defineOptions();

	return goptions;
}
}