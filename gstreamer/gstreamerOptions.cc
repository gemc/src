// gstreamer
#include "gstreamerOptions.h"
#include "gstreamer.h"

// namespace to define options
namespace gstreamer {

    vector<GOutputDefinition> getGOutputDefinition(GOptions *gopts) {

            vector<GOutputDefinition> goutputs;

            auto goutput_node = gopts->get_option_node("goutput");

            for (auto goutput_item: goutput_node) {
                goutputs.push_back(GOutputDefinition(
                        gopts->get_variable_in_option<string>(goutput_item, "format", goptions::NODFLT),
                        gopts->get_variable_in_option<string>(goutput_item, "name", goptions::NODFLT),
                        gopts->get_variable_in_option<string>(goutput_item, "type", goptions::NODFLT)
                ));
            }

            return goutputs;

    }


    // returns array of options definitions
    GOptions defineOptions() {

        GOptions goptions;


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

        string help = "Define a Output format and name\n";
        help += "\n";
        help += "Supported formats:\n";
        for(auto &format: GStreamer::supported_formats) {
            help += " - " + format + "\n";
        }
        help += "\n";
        help += "Output types\n";
        help += "\n";
        help += " - event: write events\n";
        help += " - stream: write frame stream\n";
        help += "-gstreamer=\"[{format: root, filename: output}, {format: jlabsro, filename: output}}]\"\n";

        vector <GVariable> gstreamer = {
                {"filename",   goptions::NODFLT, "name of output file"},
                {"format",     goptions::NODFLT, "format of output file"},
                {"type",       goptions::NODFLT, "type of output file"},

        };


        return goptions;
    }


}
