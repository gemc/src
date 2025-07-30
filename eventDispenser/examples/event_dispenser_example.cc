/**
 * \file eventDispenser_example.cpp
 * \brief Example program to test and demonstrate features of EventDispenser.
 *
 * \mainpage Event Dispenser Example
 *
 * \section intro_sec Introduction
 * This example demonstrates how to use the EventDispenser class to distribute
 * simulation events among runs. The EventDispenser is configured via a GOptions
 * object and a (possibly empty) global map of GDynamicDigitization plugins. In this
 * example, we simulate setting the total number of events manually, retrieve the
 * run events distribution, and then call processEvents() to run the events.
 *
 * \section usage_sec Usage
 * Compile this file together with the EventDispenser and related modules (GOptions,
 * GLogger, etc.). Run the program with appropriate command-line arguments.
 *
 * \n\n
 * \author \n &copy; Maurizio Ungaro
 * \author e-mail: ungaro@jlab.org
 * \n\n\n
 */

#include "eventDispenser.h"
#include "eventDispenser_options.h"
#include "goptions.h"

#include <map>
#include <string>


const std::string plugin_name = "test_gdynamic_plugin";

int main(int argc, char *argv[]) {
	// Create GOptions using eventDispenser::defineOptions() to aggregate options.
	auto gopts =std::make_shared<GOptions>(argc, argv, eventDispenser::defineOptions());

	// Create a logger for the EventDispenser module.
	auto log = std::make_unique<GLogger>(gopts, EVENTDISPENSER_LOGGER, "eventDispenser example");

	// For demonstration, create an empty global map of dynamic digitization plugins.
	// In a full simulation, this map would be populated with actual GDynamicDigitization plugins.
	auto dynamicRoutinesMap = gdynamicdigitization::dynamicRoutinesMap({plugin_name}, gopts);

	// Instantiate the EventDispenser with the GOptions and the global digitization map.
	EventDispenser eventDisp(gopts, dynamicRoutinesMap);

	// Retrieve and display the run events distribution.
	std::map<int, int> runEvents = eventDisp.getRunEvents();
	eventDisp.processEvents();

	return EXIT_SUCCESS;
}
