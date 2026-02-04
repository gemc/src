// eventDispenser
#include "eventDispenser_options.h"

// gemc
#include "gdynamicdigitization_options.h"

/**
 * \file eventDispenser_options.cc
 * \brief Defines Event Dispenser module options.
 *
 * This file implements eventDispenser::defineOptions(), which returns the option definition set
 * used to configure the Event Dispenser module and its dependencies.
 */

// namespace to define options
namespace eventDispenser {
// returns array of options definitions
GOptions defineOptions() {
	// Create the module option set. The logger name is used for consistent diagnostics.
	GOptions goptions(EVENTDISPENSER_LOGGER);

	// Option: number of events
	std::string help = "Example: -n=200\n";
	goptions.defineOption(GVariable("n", 0, "number of events to process"), help);

	// Option: run number
	// Clarify that this is the *conditions run number* (constants/TT selection), not the Geant4 internal run id.
	help = "Not to be confused with the geant4 run number - g4runno is set automatically \n \n";
	help += "Example: -run=12\n";
	goptions.defineOption(GVariable("run", 1, "sets run number"), help);

	// Option: run weights file
	help = "Text file with run number and their weights.\n \n";
	help += GTAB;
	help +=
		"The text file must have two columns: run# and weight. The weight represents the ratio of events desired for a run number.\n";
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

	goptions.defineOption(
		GVariable("run_weights", UNINITIALIZEDSTRINGQUANTITY, "File with run number and weights"),
		help
	);

	// Append options required by the dynamic digitization module.
	goptions += gdynamicdigitization::defineOptions();

	return goptions;
}
} // namespace eventDispenser
