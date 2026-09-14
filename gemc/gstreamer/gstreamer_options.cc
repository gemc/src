// gstreamer
#include "gstreamer_options.h"
#include "gstreamerConventions.h"

#include "gdynamicdigitization_options.h"
#include "gstreamer.h"
#include "gutilities.h"

// namespace to define options
namespace gstreamer {
using std::string;
using std::vector;

// Implementation summary:
// Parse the user-provided "gstreamer" option node into normalized
// GStreamerDefinition objects used later by plugin-loading helpers.
vector<GStreamerDefinition> getGStreamerDefinition(const std::shared_ptr<GOptions>& gopts) {
	vector<GStreamerDefinition> goutputs;

	if (!gopts->doesOptionExist("gstreamer")) { return goutputs; }

	// The "gstreamer" option node contains a list of output objects.
	// Each object is flattened into one GStreamerDefinition:
	//   - format   : plugin selector
	//   - filename : output base name
	//   - type     : semantic stream type, defaulting to "event"
	auto goutput_node = gopts->getOptionNode("gstreamer");

	if (!goutput_node || goutput_node.IsNull() || !goutput_node.IsSequence()) { return goutputs; }

	for (auto goutput_item : goutput_node) {
		const auto format = gutilities::convertToLowercase(
			gopts->get_required_variable_in_option<string>(goutput_item, "format"));
		goutputs.emplace_back(
			format,
			gopts->get_required_variable_in_option<string>(goutput_item, "filename"),
			gopts->get_optional_variable_in_option<string>(goutput_item, "type")
				.value_or(format == "sro" ? "stream" : "event")
		);
		auto& definition = goutputs.back();
		definition.implementation = gopts->get_variable_in_option<string>(goutput_item, "implementation", "");
		if (format == "sro" && (definition.implementation.empty() || definition.type != "stream")) {
			throw std::invalid_argument("sro requires an implementation plugin and type: stream (the default)");
		}
		if (format == "jlabsro") {
			throw std::invalid_argument("jlabsro has been replaced by sro with an implementation plugin");
		}
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
	string help = "Define output formats and filenames. It can be used to select event or stream outputs.\n";
	help        += "The file extension is added automatically based on the format.\n \n";
	help        += "\nSupported formats:\n \n";
	for (auto& format : GStreamer::supported_formats()) { help += "  - " + format + "\n"; }
	help += "\n \n";
	help += "Output types:\n \n";
	help += "\n";
	help += " - event: write events\n";
	help += " - stream: write frame time snapshots\n";
	help += "\n \n";
	help += "Example that defines two gstreamer outputs:\n \n";
	help += " -gstreamer=\"[{format: root, filename: out}, {format: csv, filename: out}]\"\n";
	help += "\n \n";
	help += "The produced files structure depends on the accumulation method used: \n \n";
	help += " - event-based digitization (like flux) will have one file for every thread, with \"_t<thread>\" appended to the filename \n";
	help += " - run-based digitization (like dosimeter) will have one file only\n";
	help += " - sro: one crate thread and output sink per crate, shared by all workers\n";
	help += "SRO requires an implementation plugin for payload framing, timing, and file encoding.\n";
	help += "Example: -gstreamer=\"[{format: sro, filename: out, implementation: experiment_sro}]\"\n";
	help += "The implementation resolves as experiment_sro.gplugin through -plugin_path / GEMC_PLUGIN_PATH.\n";
	help += "SRO defaults to type: stream. Its implementation controls crate/run filenames\n";
	help += "and incomplete frames.\n";

	// Buffer flush limit:
	// controls how many events each streamer instance may retain in memory
	// before the base class forces a flush to the backend.
	string ebuffer_help = "Number of events each streamer keeps in memory before flushing them to the\n";
	ebuffer_help        += "output file. Larger values reduce I/O frequency at the cost of more memory.\n \n";
	ebuffer_help        += "Example: -ebuffer=100\n";
	goptions.defineOption(GVariable("ebuffer", gstreamer::DEFAULT_GSTREAMER_BUFFER_FLUSH_LIMIT,
	                                "number of events kept in memory before flushing them to the filestream"), ebuffer_help);

	// Schema of each object in the -gstreamer array.
	vector<GVariable> gstreamer = {
		{"filename", goptions::REQUIRED, "name of output file. "},
		{"format", goptions::REQUIRED, "format of output file. "},
		{"type", std::nullopt, "type of output file (sro: stream, otherwise: event)"},
		{"implementation", "", "SRO implementation plugin library basename"},
	};

	goptions.defineOption("gstreamer", "define a gstreamer output", gstreamer, help);

	// Merge dependent-module options so callers can build a single composite
	// GOptions definition for applications and examples.
	goptions += gdynamicdigitization::defineOptions();

	return goptions;
}
}
