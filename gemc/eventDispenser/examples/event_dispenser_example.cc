/**
 * \file event_dispenser_example.cc
 * \brief Minimal example exercising the EventDispenser public API.
 *
 * \defgroup event_dispenser_example_group Event Dispenser Example
 *
 * \brief Minimal end-to-end usage of EventDispenser.
 *
 * \details
 * This example demonstrates the essential steps required to use EventDispenser:
 * - Build and parse configuration using eventDispenser::defineOptions().
 * - Build a dynamic digitization routine map (empty or populated depending on the application).
 * - Construct EventDispenser with the parsed options and the routine map.
 * - Optionally inspect the computed run distribution via \ref EventDispenser::getRunEvents "getRunEvents()".
 * - Trigger processing via \ref EventDispenser::processEvents "processEvents()".
 *
 * \section event_dispenser_example_intro_sec Introduction
 * EventDispenser distributes a total event count across one or more *run numbers*.
 * Each run number can represent different conditions (constants, translation tables, etc.).
 * The distribution can be:
 * - single-run: all events assigned to a user-selected run number, or
 * - weighted multi-run: events assigned according to a user-provided run-weight file.
 *
 * \section event_dispenser_example_details_sec Details
 * This example configures options via eventDispenser::defineOptions() and builds a digitization
 * routine map via gdynamicdigitization::dynamicRoutinesMap(). It then constructs EventDispenser,
 * retrieves the computed run allocation, and processes events.
 *
 * \note
 * This example is intentionally minimal. In a full application you would typically:
 * - provide real digitization plugins,
 * - configure additional options (e.g. variation),
 * - and print or validate the run allocation before dispatching events.
 *
 * \section event_dispenser_example_usage_sec Usage
 * Build this example together with the Event Dispenser module and its dependencies. Run with
 * the module options, for example:
 * - \c -n=200 -run=12
 * - \c -n=200 -run_weights=weights.txt
 *
 * \author \n &copy; Maurizio Ungaro
 * \author e-mail: ungaro@jlab.org
 */

#include "eventDispenser.h"
#include "eventDispenser_options.h"
#include "goptions.h"

#include <map>
#include <string>

const std::string plugin_name = "test_gdynamic_plugin";

/**
 * \ingroup event_dispenser_example_group
 * \brief Example entry point.
 *
 * \details
 * The example performs the following steps:
 * 1. Parse command-line options using eventDispenser::defineOptions().
 * 2. Create a digitization routine map (may be empty depending on build/runtime configuration).
 * 3. Construct EventDispenser and compute the run allocation.
 * 4. Call \ref EventDispenser::processEvents "processEvents()" to execute the per-run workflow.
 *
 * \param argc Standard command-line argument count.
 * \param argv Standard command-line argument vector.
 * \return \c EXIT_SUCCESS on success.
 */
int main(int argc, char *argv[]) {
	// Build the option definition set for this module and parse command-line arguments.
	auto gopts = std::make_shared<GOptions>(argc, argv, eventDispenser::defineOptions());

	// Create the global map of digitization routines.
	// In a full simulation, this map would contain the digitization plugins needed by the detectors.
	auto dynamicRoutinesMap = gdynamicdigitization::dynamicRoutinesMap({plugin_name}, gopts);

	// Instantiate the EventDispenser with parsed options and the digitization routine map.
	EventDispenser eventDisp(gopts, dynamicRoutinesMap);

	// Retrieve the run-to-event allocation computed during construction.
	// This can be used by applications to report expected run statistics or validate configuration.
	std::map<int, int> runEvents = eventDisp.getRunEvents();
	(void)runEvents; // suppress unused-variable warnings in minimal builds

	// Execute the processing loop: per-run initialization + event dispatch.
	eventDisp.processEvents();

	return EXIT_SUCCESS;
}
