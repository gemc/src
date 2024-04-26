#ifndef DOXYGEN_SHOULD_SKIP_THIS /* DOXYGEN_SHOULD_SKIP_THIS */

// goptions
#include "goptions.h"

NOT IMPLEMENTED YET

// a simple struct to model the detector option
namespace goptions {

	// detector
	struct sdet {
		string detector;
		string factory;
		string variation;
	};

	void from_json(const json& j, sdet& det) {
		j.at("detector").get_to(det.detector);
		j.at("factory").get_to(det.factory);
		j.at("variation").get_to(det.variation);
	}

	// groupable: method to return a vector<sdet>
	vector<sdet> getDetectors(GOptions *gopts) {

		vector<sdet> detectors;

		// looking over each jValues of the vector<json> options
		for (const auto& jdet: (*gopts)["detector"]) {
			detectors.push_back(jdet.get<sdet>());
		}

		return detectors;
	}

	// run number
	struct run {
		int runno;
	};

	void from_json(const json& j, run& r) {
		j.at("runno").get_to(r.runno);
	}

	// non groupable: method to return a single run
	run getRun(vector<json> jValues) {
		return jValues.front().get<run>();
	}

}



// returns array of options definitions
vector<GOption> defineOptions()
{
	vector<GOption> goptions;

	// detector option
	// groupable: can use -add
	json jdetectorTag = {
		{GNAME, "detector"},
		{GDESC, "detector system name. For TEXT factories, it includes the path to the file."},
		{GDFLT, NODFLT}
	};
	json factoryTag = {
		{GNAME, "factory"},
		{GDESC, "detector factory name"},
		{GDFLT, NODFLT}
	};
	json variationTag = {
		{GNAME, "variation"},
		{GDESC, "detector variation."},
		{GDFLT, "spring2019"}
	};

	json jDetOptionDefinitions = { jdetectorTag, factoryTag, variationTag};

	goptions.push_back(GOption("detector", "detector option", jDetOptionDefinitions, true, "help example"));

	// run option
	// not groupable
	json runTag = {
		{GNAME, "runno"},
		{GDESC, "Run number"},
		{GDFLT, 0}
	};

	goptions.push_back(GOption("runno", "run option", runTag));


	// activate gui. Default = no (batch mode)
	json guiTag = {
		{GNAME, "gui"},
		{GDESC, "Graphical User Interface. Possible Values: 0/1. Default: 0"},
		{GDFLT, 0}
	};

	// number of threads. Default = 1
	json nthreadsTag = {
		{GNAME, "nthreads"},
		{GDESC, "Number of threads"},
		{GDFLT, 1}
	};

	json jConfTag = { guiTag, nthreadsTag};

	goptions.push_back(GOption("gConf", "GEMC Configuration: gui, number of threads", jConfTag));

	return goptions;
}


int main(int argc, char* argv[])
{
	GOptions *gopts = new GOptions(argc, argv, defineOptions());

	// print settings w/o defaults
	gopts->printSettings(false);

	// Perhaps there's a better modern way to do this
	vector<goptions::sdet> detectors = goptions::getDetectors(gopts);

	for (auto& det: detectors) {
		cout << " detector  " << det.detector << ", factory  " << det.factory << ", variation  " << det.variation << endl;
	}

	return EXIT_SUCCESS;
}

#endif /* DOXYGEN_SHOULD_SKIP_THIS */
