// gstreamer
#include "gstreamer_options.h"
#include "gstreamerConventions.h"

#include "gdynamicdigitization_options.h"
#include "gstreamer.h"

// namespace to define options
namespace gstreamer {

using std::string;
using std::vector;

vector<GStreamerDefinition> getGStreamerDefinition(const std::shared_ptr<GOptions>& gopts) {
	vector<GStreamerDefinition> goutputs;

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


// returns array of options definitions
GOptions defineOptions() {
	GOptions goptions(GSTREAMER_LOGGER);

	string help = "Define a Output format and name\n";
	help += "This option defines a GStreamer output. It can be used to write events or frame streams.\n";
	help += "\n";
	help += "The file extension is added automatically based on the format.\n";
	help += "Supported formats:\n";
	for (auto& format : GStreamer::supported_formats()) { help += " - " + format + "\n"; }
	help += "\n";
	help += "Output types\n";
	help += "\n";
	help += " - event: write events\n";
	help += " - stream: write frame stream\n";
	help += "\n";
	help += "Example that defines two gstreamer outputs:\n";
	help += "-gstreamer=\"[{format: root, filename: out}, {format: jlabsro, filename: out}}]\"\n";

	// buffer
	goptions.defineOption(GVariable("ebuffer", DEFAULT_GSTREAMER_BUFFER_FLUSH_LIMIT, "number of events kept in memory before flushing them to the filestream"), help);

	vector<GVariable> gstreamer = {
		{"filename", goptions::NODFLT, "name of output file"},
		{"format", goptions::NODFLT, "format of output file"},
		{"type", "event", "type of output file"},

	};

	goptions.defineOption(GSTREAMER_LOGGER, "define a gstreamer output", gstreamer, help);

	goptions += gdynamicdigitization::defineOptions();

	return goptions;
}


}
