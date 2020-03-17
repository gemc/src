// options definitions
#include "defineOptions.h"

// c++
#include <iostream>
using namespace std;

namespace goptions {

	void from_json(const json& j, GConf& c) {
		j.at("gui").get_to(c.gui);
		j.at("nthreads").get_to(c.nthreads);
		j.at("stageMessageVerbosity").get_to(c.stageMessageVerbosity);
	}

	// non groupable: method to return a single gui
	GConf getGConf(GOptions *gopts) {
		auto jConf = (*gopts)["conf"].front();
		return jConf.get<GConf>();
	}

	bool getGui(GOptions *gopts) {
		auto gConf = getGConf(gopts);
		return gConf.gui == 1;
	}

	int getSMV(GOptions *gopts) {
		auto gConf = getGConf(gopts);
		return gConf.stageMessageVerbosity;
	}

	int getVerbosity(GOptions *gopts, string state) {
		auto jConf = (*gopts)["gConf"].front();

		cout << " ASD " << jConf << endl;

		return jConf[state];
	}

}

// returns array of options definitions
vector<GOption> defineOptions()
{
	vector<GOption> goptions;

	// activate gui. Default = no (batch mode)
	json guiTag = {
		{JSONTAGNAME, "gui"},
		{JSONTAGDESC, "Graphical User Interface. Possible Values: 0/1. Default: 0"},
		{JSONTAGDFLT, 0}
	};

	// number of threads. Default = 1
	json nthreadsTag = {
		{JSONTAGNAME, "nthreads"},
		{JSONTAGDESC, "Number of threads"},
		{JSONTAGDFLT, 1}
	};

	// stage message verbosity
	json smvTag = {
		{JSONTAGNAME, "stageMessageVerbosity"},
		{JSONTAGDESC, "Verbosity of State Messages"},
		{JSONTAGDFLT, 1}
	};



	json jConfTag = { guiTag, nthreadsTag, smvTag};
	string help = "GEMC configuration options.";

	goptions.push_back(GOption("conf", "GEMC Configuration: gui, number of threads", jConfTag, help));

	return goptions;
}




