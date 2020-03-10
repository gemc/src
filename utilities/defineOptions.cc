// goptions
#include "goptions.h"




//// methods to return single options
//// GConf gui as book
//bool getGui(GOptions *gopts) {
//
//	vector<json> gConf = gopts->getOptions("gui");
//	goptions::gui gui = goptions::getGui(guiOptions);
//
//
//	return gui.gui == 1;
//}




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


	json jConfTag = { guiTag, nthreadsTag};


//	goptions.push_back(GOption("gConf", "GEMC Configuration: gui, number of threads", jConfTag));



	return goptions;
}




