// goptions
#include "goptions.h"

// options definitions
#include "defineOptions.h"

// c++
#include <iostream>
using namespace std;


namespace goptions {

	void from_json(const json& j, GemcConfiguration& c) {
		j.at("gui").get_to(c.gui);
		j.at("nthreads").get_to(c.nthreads);
		j.at("stageMessageVerbosity").get_to(c.stageMessageVerbosity);
	}

	// get the configuration from options
	GemcConfiguration getGemcConfiguuration(GOptions *gopts) {
		auto jsonConfigurations = gopts->getOption("GemcConfiguration");
		if ( jsonConfigurations.size()) {
			return jsonConfigurations.front().get<GemcConfiguration>();
		} else {
			exit(1);
		}
	}

}

// returns array of options definitions
vector<GOption> defineOptions()
{
	vector<GOption> goptions;

	// activate gui. Default = no (batch mode)
	json jsonGuiOption = {
		{GNAME, "gui"},
		{GDESC, "Graphical User Interface. Possible Values: 0/1. Default: 0"},
		{GDFLT, 0}
	};

	// number of threads. Default = 1
	json jsonNThreadOption = {
		{GNAME, "nthreads"},
		{GDESC, "Number of threads"},
		{GDFLT, 1}
	};

	// stage message verbosity
	json jsonMessageOption = {
		{GNAME, "messageVerbosity"},
		{GDESC, "Verbosity of GEMC State Messages"},
		{GDFLT, 1}
	};



	json jConfTag = { jsonGuiOption, jsonNThreadOption, jsonMessageOption};
	string help = "GEMC configuration option.";

	goptions.push_back(GOption("GemcConfiguration", "GEMC Configuration: gui, number of threads", jConfTag, help));

	return goptions;
}


