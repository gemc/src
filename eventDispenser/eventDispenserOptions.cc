// eventDispenser
#include "eventDispenserOptions.h"

// glibrary
#include "gutsConventions.h"

// namespace to define options
namespace eventDispenser {

// returns array of options definitions
vector<GOption> defineOptions() {
	
	vector<GOption> goptions;
	
	json jsonEDVerbosity = {
		{GNAME, "edistv"},
        {GDESC, "Verbosity for eventDispenser. " + string(GVERBOSITY_DESCRIPTION)},

        {GDFLT, 0}
	};
	goptions.push_back(GOption(jsonEDVerbosity));
	
	string whelp = "Text file with run number and their weights\n";
	whelp += GTAB;
	whelp += "The text file must have two columns, run# and weight.\n";
	whelp += GTAB;
	whelp += "For example a \"weights.txt\" file that contains:\n";
	whelp += GTAB;
	whelp += "11 0.1\n";
	whelp += GTAB;
	whelp += "12 0.7\n";
	whelp += GTAB;
	whelp += "13 0.2\n";
	whelp += GTAB;
	whelp += "will simulate 10% of events with run number 11 conditions, 70% for run 12 and 20% for run 13.\n";
	json jsonRunWeights = {
		{GNAME, "runWeightsFile"},
		{GDESC, whelp},
		{GDFLT, UNINITIALIZEDSTRINGQUANTITY}
	};
	goptions.push_back(GOption(jsonRunWeights));
	
	
	json jsonNEvents = {
		{GNAME, "n"},
		{GDESC, "Number of events to process"},
		{GDFLT, 0}
	};
	goptions.push_back(GOption(jsonNEvents));
	
	json jsonUserRunno = {
		{GNAME, "userRunno"},
		{GDESC, "User Run Number"},
		{GDFLT, 1}
	};
	goptions.push_back(GOption(jsonUserRunno));
	
	json jsonNEventsBuffer = {
		{GNAME, "maxebuffer"},
		{GDESC, "Max number of events to keep in memory before writing out the output."},
		{GDFLT, 100}
	};
	goptions.push_back(GOption(jsonNEventsBuffer));
	
	return goptions;
}

}
