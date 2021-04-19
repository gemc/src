// goptions
#include "goptions.h"

// options definitions
#include "defineOptions.h"

// c++
#include <iostream>
using namespace std;


namespace goptions {


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
	goptions.push_back(GOption(jsonGuiOption));

	// number of threads. Default = 1
	json jsonNThreadOption = {
		{GNAME, "nthreads"},
		{GDESC, "Number of threads"},
		{GDFLT, 1}
	};
	goptions.push_back(GOption(jsonNThreadOption));

	// stage message verbosity
	json jsonMessageOption = {
		{GNAME, "verbosity"},
		{GDESC, "Verbosity of GEMC State Messages"},
		{GDFLT, 1}
	};
	goptions.push_back(GOption(jsonMessageOption));


	return goptions;
}


