// goptions
#include "goptions.h"

// a simple struct to model the detector option
namespace goptions {

	// single option to activate gui
	// -----------------------------

	struct gui {
		int gui;
	};

	void from_json(const json& j, gui& g) {
		j.at("gui").get_to(g.gui);
	}

	// non groupable: method to return a single gui
	gui getGui(vector<json> jValues) {
		return jValues.front().get<gui>();
	}


}




// methods to return single options
bool getGui(GOptions *gopts) {


	vector<json> guiOptions = gopts->getOptions("gui");
	goptions::gui gui = goptions::getGui(guiOptions);


	return gui.gui == 1;
}









// returns array of options definitions
vector<GOption> defineOptions()
{
	vector<GOption> goptions;

	// single option to activate gui
	// -----------------------------
	json guiTag = {
		{JSONTAGNAME, "gui"},
		{JSONTAGDESC, "Graphical User Interface. Possible Values: 0/1. Default: 0"},
		{JSONTAGDFLT, 0}
	};

	goptions.push_back(GOption("gui", "Graphical User Interface. Possible Values: 0/1. Default: 0", guiTag));

	return goptions;
}




