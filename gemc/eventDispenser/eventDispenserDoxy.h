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
 * @section options_sec Available Options and their usage
 *
 * This module reads the following option keys from the runtime option provider:
 *
 * - `n`
 *   - Type: integer
 *   - Meaning: requested number of events to process
 *   - Behavior:
 *     - `0` typically means "no events requested" (module may still initialize and report configuration)
 *     - positive values are used to compute the per-run event allocation
 *
 * - `run`
 *   - Type: integer
 *   - Meaning: conditions run number used when no run-weight file is provided
 *   - Behavior:
 *     - used as the single run number for the full event set when `run_weights` is unset
 *   - Note: not to be confused with the Geant4 internal run id (\c g4runno), which GEMC manages separately.
 *
 * - `run_weights`
 *   - Type: string (path)
 *   - Meaning: text file containing run numbers and relative weights
 *   - Behavior:
 *     - when set, the module reads the file and distributes events across the listed run numbers
 *     - weights are interpreted as relative ratios and normalized internally
 *   - File format:
 *     - two columns: `<run_number> <weight>`
 *     - example:
 *       - `11 0.1`
 *       - `12 0.7`
 *       - `13 0.2`
 *
 * This module’s option schema is composed by \c eventDispenser::defineOptions(), which aggregates:
 * - \c gdynamicdigitization::defineOptions()
 *
 * Additional keys contributed by \c gdynamicdigitization::defineOptions() affect digitization selection
 * and per-run initialization behavior. Refer to that module’s documentation for the full list.
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
