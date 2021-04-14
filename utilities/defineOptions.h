#ifndef DEFINEOPTIONS_H
#define DEFINEOPTIONS_H 1

// goptions
#include "goptions.h"

namespace goptions {

	// single option to activate gui
	// -----------------------------

	struct GConf {
		int gui;
		int nthreads;
		int stageMessageVerbosity;
	};

	//void from_json(const json& j, GConf& c);

	// non groupable: method to return a single gui
	//GConf getGConf(GOptions *gopts);

	//bool getGui(GOptions *gopts);
	//int getSMV(GOptions *gopts);
	//int getVerbosity(GOptions *gopts, string state);

}

// returns array of options definitions
// this calls all the static defineOptions functions in the various libraries/frameworks
vector<GOption> defineOptions();

#endif


