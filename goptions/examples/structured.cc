#ifndef DOXYGEN_SHOULD_SKIP_THIS /* DOXYGEN_SHOULD_SKIP_THIS */

// goptions
#include "goptions.h"

// gutilities
#include "gutilities.h"
using namespace gutilities;

// c++
#include <iostream>
using namespace std;


// a simple struct to project the gdetector structured option
// onto a GDetector C structure
namespace gdetector {
	
	struct GDetector {
		string detector;
		string factory;
		string variation;
	};

	void from_json(const json& j, GDetector& det) {
		//auto jdet = j.at("gdetector");
		j.at("detector").get_to(det.detector);
		j.at("factory").get_to(det.factory);
		j.at("variation").get_to(det.variation);
	}

	// method to return a GDetectors from a structured option
	GDetector getDetector(GOptions *gopts) {

		// getting json detector from option
		auto gdet = gopts->getStructuredOptionAssignedValues("gdetector");

		// projecting it onto GDetector structure
		return gdet.front().get<GDetector>();
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

		goptions.push_back(GOption("gdetector", "Adds a detector", jsonDetectorOption, help));

		return goptions;
	}

}

using gdetector::getDetector;
using gdetector::GDetector;


// example of parsing one jcard and reading a variable
int main(int argc, char* argv[])
{
	GOptions *gopts = new GOptions(argc, argv, gdetector::defineOptions());

	// print settings with defaults
	gopts->printSettings(true);

	// projecting option onto GDetector
	GDetector gdet = getDetector(gopts);

	cout << " Accessing projected structure GDetector: " << endl << endl;
	cout << " detector: " << gdet.detector << ", factory: " << gdet.factory << ", variation: " << gdet.variation << endl;
	cout << endl;

    if (gdet.detector == "experiments/clas12" && gdet.factory == "SQLITE" && gdet.variation == "rga_fall2018") {
        return EXIT_SUCCESS;
    }

	return EXIT_FAILURE;
}

#endif /* DOXYGEN_SHOULD_SKIP_THIS */
