/**
 * \file eventDispenserDoxy.h
 * \brief Doxygen mainpage for the Event Dispenser module.
 */

/**
 *
 * \mainpage Event Dispenser Module
 *
 * \section intro_sec Introduction
 * The Event Dispenser module distributes simulation events across one or more *run numbers*.
 * The distribution is driven either by a single user-selected run number, or by a file that
 * specifies multiple runs and their relative weights.
 *
 * \section details_sec Details
 * At runtime, EventDispenser:
 * - Reads user configuration from a GOptions instance.
 * - Builds an internal run list and a run-to-weight map when a run-weight file is provided.
 * - Derives a per-run event allocation by sampling a uniform random variable against the
 *   cumulative weight distribution.
 * - For each run, initializes (or re-initializes) the available GDynamicDigitization routines
 *   so each run can load its run-dependent constants and translation tables.
 * - Dispatches the actual event generation to Geant4 via UI commands (e.g. \c /run/beamOn).
 *
 * \section usage_sec Typical usage
 * A typical application flow is:
 * - Construct GOptions with the module options from eventDispenser::defineOptions().
 * - Construct EventDispenser with the parsed options and the map of digitization routines.
 * - Query the computed distribution with \ref EventDispenser::getRunEvents "getRunEvents()".
 * - Run the workflow using \ref EventDispenser::processEvents "processEvents()".
 *
 * \section verbosity_sec Module verbosity
 * EventDispenser uses the standard GEMC logging infrastructure (classes derived from glogger).
 * The practical meaning of verbosity levels is:
 * - **info level 0**: high-level summary (initialization and final distribution table).
 * - **info level 1**: per-run progress messages (start/end of each run).
 * - **info level 2**: additional operational detail (reserved for more verbose progress).
 *
 * Debug output (enabled through the logger configuration) prints *diagnostic* information such as:
 * - which digitization routine is being called,
 * - which run number and variation are being used for constants / translation-table loading,
 * - and other function-level traces useful during development.
 *
 * \section examples_sec Examples
 * - \ref event_dispenser_example_group "Event Dispenser Example" :
 *   Minimal end-to-end usage showing how to create options, instantiate EventDispenser, query the
 *   run distribution, and trigger processing.
 *
 * \code
 * auto gopts = std::make_shared<GOptions>(argc, argv, eventDispenser::defineOptions());
 * EventDispenser eventDisp(gopts, dynamicRoutinesMap);
 * auto runEvents = eventDisp.getRunEvents();
 * eventDisp.processEvents();
 * \endcode
 *
 * \n\n
 * \author \n &copy; Maurizio Ungaro
 * \author e-mail: ungaro@jlab.org
 * \n\n\n
 */
