// goptions
#include "goptions.h"

// options definitions
#include "defineOptions.h"

// c++
#include <iostream>
using namespace std;


namespace gemc {

	// returns array of options definitions
	vector<GOption> defineOptions()
	{
		vector<GOption> goptions;

		// add a "gui" switch
		goptions.push_back(GOption("gui", "Use Graphical User Interface"));

		// number of threads. Default = 1
		json jsonNThreadOption = {
			{GNAME, "nthreads"},
			{GDESC, "Number of threads"},
			{GDFLT, 0}
		};
		goptions.push_back(GOption(jsonNThreadOption));

		// stage message verbosity
		json jsonMessageOption = {
			{GNAME, "verbosity"},
			{GDESC, "Verbosity of GEMC State Messages. Possible values: 0 (none), 1 (some), 2 (all)"},
			{GDFLT, 1}
		};
		goptions.push_back(GOption(jsonMessageOption));


		return goptions;
	}


}



