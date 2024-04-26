// gstreamer
#include "gstreamerOptions.h"

// namespace to define options
namespace gstreamer {


// projecting options onto JOutput
void from_json(const json& j, JOutput& output) {
	j.at("format").get_to(output.format);
	j.at("name").get_to(output.name);
	j.at("type").get_to(output.type);
}

// method to return a vector of GDetectors from a structured option
vector<JOutput> getJOutputs(GOptions *gopts) {
	
	vector<JOutput> outputs;
	
	auto gouts = gopts->getStructuredOptionAssignedValues("goutput");
	
	// looking over each of the vector<json> items
	for ( const auto& gout: gouts ) {
		outputs.push_back(gout.get<JOutput>());
	}
	
	return outputs;
}

// returns dynamic library name from the factory
string gstreamerPluginNameFromFactory(string factory) {
	
	return "gstreamer" + factory + "Factory";
}

// returns array of options definitions
vector<GOption> defineOptions() {
	
	vector<GOption> goptions;
	
	// two verbosity: one for events, one for streaming
	json jsonGStreamerEventVerbosity = {
		{GNAME, "geventstreamv"},
        {GDESC, "Verbosity for gstreamer. " + string(GVERBOSITY_DESCRIPTION)},
		{GDFLT, 0}
	};
	goptions.push_back(GOption(jsonGStreamerEventVerbosity));
	
	json jsonGStreamerFrameVerbosity = {
		{GNAME, "gframestreamv"},
		{GDESC, GVERBOSITY_DESCRIPTION},
		{GDFLT, 0}
	};
	goptions.push_back(GOption(jsonGStreamerFrameVerbosity));
	
	// default material to use when a material is not found
	json jsonOutputFormat = {
		{GNAME, "format"},
		{GDESC, "Output file format"},
		{GDFLT, UNINITIALIZEDSTRINGQUANTITY}
	};
	json jsonOutputName = {
		{GNAME, "name"},
		{GDESC, "Output file name"},
		{GDFLT, UNINITIALIZEDSTRINGQUANTITY}
	};
	json jsonOutputType = {
		{GNAME, "type"},
		{GDESC, "Output type"},
		{GDFLT, UNINITIALIZEDSTRINGQUANTITY}
	};
	
	json jsonOutput = {
		jsonOutputFormat,
		jsonOutputName,
		jsonOutputType
	};
	
	vector<string> help;
	help.push_back("Define a Output format and name");
	help.push_back("");
	help.push_back("Example: +output={format: \"TEXT\", name: \"output.txt\", type: \"event\" }");
	help.push_back("");
	help.push_back("Current available formats:");
	help.push_back("");
	help.push_back(" - TEXT");
	help.push_back("");
	help.push_back("Output types");
	help.push_back("");
	help.push_back(" - event: write events");
	help.push_back(" - stream: write frame stream");
	
	// the last argument refers to "cumulative"
	goptions.push_back(GOption("goutput", "Output format and name", jsonOutput, help, true));
	
	return goptions;
}


}
