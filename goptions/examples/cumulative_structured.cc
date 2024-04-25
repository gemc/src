#ifndef DOXYGEN_SHOULD_SKIP_THIS /* DOXYGEN_SHOULD_SKIP_THIS */

// goptions
#include "goptions.h"

// gutilities
#include "gutilities.h"
using namespace gutilities;

// c++
#include <iostream>
using namespace std;


// a simple struct to project the gsetup cumulative structured option
// onto a vector of GDetector C structures
namespace gdetector {
	
	struct GDetector {
		string detector;
		string factory;
		string variation;
	};

	void from_json(const json& j, GDetector& det) {
		j.at("detector").get_to(det.detector);
		j.at("factory").get_to(det.factory);
		j.at("variation").get_to(det.variation);
	}

	// method to return a vector of GDetectors from a structured option
	vector<GDetector> getDetectors(GOptions *gopts) {

		vector<GDetector> detectors;

		auto gdets = gopts->getStructuredOptionAssignedValues("gsetup");

		// looking over each of the vector<json> items
		for ( const auto& gdet: gdets ) {
			detectors.push_back(gdet.get<GDetector>());
		}

		return detectors;
	}

	// returns array of options definitions
	vector<GOption> defineOptions()
	{
		vector<GOption> goptions;

		// detector option
		// groupable: can use -add
		json jsonDetectorTag = {
			{GNAME, "detector"},
			{GDESC, "detector system name. For TEXT factories, it includes the path to the file."},
			{GDFLT, NODFLT}
		};
		json jsonFactoryTag = {
			{GNAME, "factory"},
			{GDESC, "detector factory name"},
			{GDFLT, NODFLT}
		};
		json jsonVariationTag = {
			{GNAME, "variation"},
			{GDESC, "detector variation."},
			{GDFLT, "default"}
		};

		json jsonDetectorOption = { jsonDetectorTag, jsonFactoryTag, jsonVariationTag};

		vector<string> help;

		help.push_back("A detector definition includes the geometry location, factory and variation");
		help.push_back("The geometry and variation are mandatory fields");
		help.push_back("The variation is optional, with \"default\" as default");
		help.push_back("");
		help.push_back("Example: +gsetup={detector: \"experiments/clas12/targets\", factory: \"TEXT\", \"variation\": \"bonus\"}");

		// the last argument refers to "cumulative"
		goptions.push_back(GOption("gsetup", "define a group of detectors", jsonDetectorOption, help, true));

		return goptions;
	}

}

using gdetector::getDetectors;
using gdetector::GDetector;

// example of parsing one jcard and reading a variable
int main(int argc, char* argv[])
{
	GOptions *gopts = new GOptions(argc, argv, gdetector::defineOptions());

	// print settings with defaults
	gopts->printSettings(true);

	// projecting option onto vector of GDetectors
	vector<GDetector> detectors = getDetectors(gopts);

	if (detectors.size()) {
		cout << " Accessing projected structure: " << endl << endl;
		for (auto& det: detectors) {
			cout << " detector: " << det.detector << ",\t factory: " << det.factory << ",\t variation: " << det.variation << endl;
		}
		cout << endl;
	}
	
	return EXIT_SUCCESS;
}

#endif /* DOXYGEN_SHOULD_SKIP_THIS */
