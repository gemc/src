// goptions
#include "goptions.h"

// c++
#include <iostream>
using namespace std;


namespace goptions {

	// single option to activate gui
	// -----------------------------

	struct GemcConfiguration {
		int gui;
		int nthreads;
		int stageMessageVerbosity;
	};


	void from_json(const json& j, GemcConfiguration& c) {
		j.at("gui").get_to(c.gui);
		j.at("nthreads").get_to(c.nthreads);
		j.at("stageMessageVerbosity").get_to(c.stageMessageVerbosity);
	}

	// get the configuration from options
	GemcConfiguration getGemcConfiguuration(GOptions *gopts) {
		auto jsonConfiguration = gopts->getOption("GemcConfiguration").front();
		return jsonConfiguration.get<GemcConfiguration>();
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


