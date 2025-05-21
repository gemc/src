// gstreamer
#include "gstreamer_options.h"
#include "gstreamer.h"

// namespace to define options
namespace gstreamer {

using std::string;
using std::vector;

vector<GStreamerDefinition> getGStreamerDefinition(GOptions* gopts) {
	vector<GStreamerDefinition> goutputs;

	auto goutput_node = gopts->getOptionNode("gstreamer");

	for (auto goutput_item : goutput_node) {
		goutputs.emplace_back(GStreamerDefinition(
		                                          gopts->get_variable_in_option<string>(goutput_item, "format", goptions::NODFLT),
		                                          gopts->get_variable_in_option<string>(goutput_item, "filename", goptions::NODFLT),
		                                          gopts->get_variable_in_option<string>(goutput_item, "type", "event")
		                                         ));
	}

	return goutputs;
}


// returns array of options definitions
GOptions defineOptions() {
	GOptions goptions(GSTREAMER_LOGGER);

	string help = "Define a Output format and name\n";
	help += "\n";
	help += "Supported formats:\n";
	for (auto& format : GStreamer::supported_formats) { help += " - " + format + "\n"; }
	help += "\n";
	help += "Output types\n";
	help += "\n";
	help += " - event: write events\n";
	help += " - stream: write frame stream\n";
	help += "-gstreamer=\"[{format: root, filename: output}, {format: jlabsro, filename: output}}]\"\n";

	vector<GVariable> gstreamer = {
		{"filename", goptions::NODFLT, "name of output file"},
		{"format", goptions::NODFLT, "format of output file"},
		{"type", "event", "type of output file"},

	};

	goptions.defineOption(GSTREAMER_LOGGER, "define a gstreamer output", gstreamer, help);


	return goptions;
}


}
