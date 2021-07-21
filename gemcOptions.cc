// goptions
#include "goptions.h"

// options definitions
#include "gemcOptions.h"

// other modules goptions
#include "gsystemOptions.h"
#include "g4systemOptions.h"


// c++
#include <iostream>
using namespace std;


namespace gemc {

	// returns array of options definitions
	vector<GOption> defineOptions()
	{
		vector<GOption> goptions;

		// add a "gui" switch
		goptions.push_back(GOption("gui", "use Graphical User Interface"));

		// number of threads. Default = 1
		json jsonNThreadOption = {
			{GNAME, "nthreads"},
			{GDESC, "set number of threads"},
			{GDFLT, 0}
		};
		goptions.push_back(GOption(jsonNThreadOption));

		// general verbosity
		json jsonMessageVerbosity = {
			{GNAME, "verbosity"},
			{GDESC, GVERBOSITY_DESCRIPTION},
			{GDFLT, 1}
		};
		goptions.push_back(GOption(jsonMessageVerbosity));


		// other modules goptions
		goptions += gsystem::defineOptions();
		goptions += g4system::defineOptions();


		return goptions;
	}


}



