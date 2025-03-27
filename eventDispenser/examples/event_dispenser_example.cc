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
#include "glogger.h"

#include <iostream>
#include <map>
#include <string>

using namespace std;

int main(int argc, char *argv[]) {
	// Create GOptions using eventDispenser::defineOptions() to aggregate options.
	GOptions *gopts = new GOptions(argc, argv, eventDispenser::defineOptions());

	// Create a logger for the EventDispenser module.
	GLogger *log = new GLogger(gopts, EVENTDISPENSER_LOGGER);

	// For demonstration, create an empty global map of dynamic digitization plugins.
	// In a full simulation, this map would be populated with actual GDynamicDigitization plugins.
	map<string, GDynamicDigitization *> *gDynamicMap = new map<string, GDynamicDigitization *>();

	// Instantiate the EventDispenser with the GOptions and the global digitization map.
	EventDispenser eventDisp(gopts, gDynamicMap);

	// Retrieve and display the run events distribution.
	map<int, int> runEvents = eventDisp.getRunEvents();


	eventDisp.processEvents();

	// Cleanup: delete dynamically allocated objects.
	delete gopts;
	delete log;
	delete gDynamicMap;

	return EXIT_SUCCESS;
}
