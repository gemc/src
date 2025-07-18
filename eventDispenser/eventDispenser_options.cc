// eventDispenser
#include "eventDispenser_options.h"

// gemc
#include "gdynamicdigitization_options.h"

// namespace to define options
namespace eventDispenser {

// returns array of options definitions
GOptions defineOptions() {
	GOptions goptions(EVENTDISPENSER_LOGGER);

	std::string help = "Example: -n=200\n";
	goptions.defineOption(GVariable("n", 0, "number of events to process"), help);

	// runno: 12
	help = "Not to be confused with the geant4 run number - g4runno is set automatically \n \n";
	help += "Example: -run=12\n";
	goptions.defineOption(GVariable("run", 1, "sets run number"), help);

	help = "Text file with run number and their weights.\n \n";
	help += GTAB;
	help += "The text file must have two columns: run# and weight. The weight represents the ratio of events desired for a run number.\n";
	help += GTAB;
	help += "For example a \"weights.txt\" file that contains:\n \n";
	help += GTABTAB;
	help += "11 0.1\n";
	help += GTABTAB;
	help += "12 0.7\n";
	help += GTABTAB;
	help += "13 0.2\n \n";
	help += GTAB;
	help += "will simulate 10% of events with run number 11 conditions, 70% for run 12 and 20% for run 13.\n";
	goptions.defineOption(GVariable("run_weights", UNINITIALIZEDSTRINGQUANTITY, "File with run number and weights"),
	                      help);

	help = "Max number of events  to buffer before they're written out.\n \n";
	help += "Example: -n_event_buffer=12\n";
	goptions.defineOption(GVariable("n_event_buffer", 100, "Buffer events and write them out in one file"), help);
	// TODO: change to 0, and write out events in each thread as default
	// TODO: maybe this will replace the "stream" switch?
	// help = "By default gemc writes one output file / thread. If the 'stream' swith is enabled, .\n \n";
	// help += "Example: -n_event_buffer=12\n";
	// goptions.defineOption(GVariable("n_event_buffer", 0, "Buffer events and write them out in one file"), help);

	goptions += gdynamicdigitization::defineOptions();


	return goptions;
}

}
