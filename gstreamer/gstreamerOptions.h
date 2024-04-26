#ifndef  GSTREAMEROPTIONS_H
#define  GSTREAMEROPTIONS_H 1

// glibrary
#include "goptions.h"

namespace gstreamer {

struct JOutput {
	string format;
	string name;
	string type;
};

void from_json(const json& j, JOutput& det);

// returns dynamic library name from the factory
string gstreamerPluginNameFromFactory(string factory);

// method to return a vector of JSystem from a structured option
vector<JOutput> getJOutputs(GOptions *gopts);

// returns array of options definitions
vector<GOption> defineOptions();

}


#endif
