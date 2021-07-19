// goptions
#include "goptions.h"

// options definitions
#include "gemcOptions.h"

// other systems options
#include "gsystemOptions.h"


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

		// stage message verbosity
		json jsonMessageVerbosity = {
			{GNAME, "verbosity"},
			{GDESC, GVERBOSITY_DESCRIPTION},
			{GDFLT, 1}
		};
		goptions.push_back(GOption(jsonMessageVerbosity));


		// geant4 world verbosity is defined here and passed to the g4world constructor
		json jsonG4WorldVerbosity = {
			{GNAME, "g4worldv"},
			{GDESC, GVERBOSITY_DESCRIPTION},
			{GDFLT, 1}
		};
		goptions.push_back(GOption(jsonG4WorldVerbosity));

		goptions += gsystem::defineOptions();


		return goptions;
	}


}



