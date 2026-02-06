/**
 * \mainpage GEMC Utilities Module
 *
 * @section intro_sec Introduction
 *
 * The GEMC Utilities Module provides small, focused runtime helpers used during
 * application startup and configuration. These helpers are designed to keep the
 * initialization logic:
 * - readable (explicit command lists rather than ad-hoc scattered calls),
 * - reproducible (commands are built deterministically from options),
 * - and easy to diagnose (key actions are logged).
 *
 * The utilities primarily support:
 * - creation of the Qt application object in GUI or batch mode,
 * - selection of the number of worker threads,
 * - preparation and execution of Geant4 UI command sequences,
 * - selection and seeding of the random engine.
 *
 * @section ownership_sec Ownership and lifecycle
 *
 * The utilities are predominantly **stateless**: functions either compute values
 * (such as thread counts), build command lists, or apply command lists.
 *
 * Key ownership points:
 * - The Qt application object created by `gemc::makeQtApplication()` is owned by the
 *   returned `std::unique_ptr<QCoreApplication>`.
 * - In batch mode, `gemc::makeQtApplication()` creates a `GBatch_Session` and transfers
 *   ownership to Geant4 via `G4UImanager::SetCoutDestination`. The caller must not
 *   attempt to delete that session object.
 *
 * @section arch_sec Architecture
 *
 * @subsection design_notes Design notes
 *
 * Initialization is intentionally split into two phases:
 * 1. **Command construction**: helpers such as `gemc::verbosity_commands()` and
 *    `gemc::initial_commands()` produce ordered lists of UI commands as `std::vector<std::string>`.
 * 2. **Command execution**: `gemc::run_manager_commands()` applies those commands to the UI manager.
 *
 * This separation provides:
 * - a single place to reason about which commands are executed and in what order,
 * - the ability to log and/or inspect command sequences before applying them,
 * - and predictable behavior across GUI and batch configurations.
 *
 * Random-engine configuration is handled by `gemc::start_random_engine()`, which selects the
 * engine implementation by name and seeds it from either user input or a derived seed.
 *
 * @section options_sec Available Options and their usage
 *
 * This module reads the following option keys from the runtime option provider:
 *
 * - `nthreads`
 *   - Type: integer
 *   - Meaning: requested number of worker threads
 *   - Behavior:
 *     - `0` means "use all available cores"
 *     - values larger than the detected core count are clamped
 *
 * - `check_overlaps`
 *   - Type: integer
 *   - Meaning: request for geometry overlap checks before initialization
 *   - Behavior:
 *     - `2` triggers a test run with 50 points
 *     - values `>= 100` trigger a test run with that many points
 *   - Note: this option is commonly provided by the Geant4 system options set.
 *
 * - `gui`
 *   - Type: boolean (switch)
 *   - Meaning: enable visualization / GUI initialization sequence
 *   - Behavior:
 *     - when `false`, `gemc::initial_commands()` only returns the non-visual initialization commands
 *     - when `true`, visualization setup commands are appended
 *
 * - `randomEngine`
 *   - Type: string
 *   - Meaning: name of the random engine implementation to instantiate
 *   - Behavior: engine selection is performed by string match inside `gemc::start_random_engine()`
 *
 * - `seed`
 *   - Type: integer
 *   - Meaning: numeric seed used for the selected engine
 *   - Behavior:
 *     - if unset (equal to `SEEDNOTSET`), a seed is derived from time/clock/pid sources
 *
 * @section verbosity_sec Module verbosity
 *
 * The utilities report their activity through the logger passed into each function.
 * A typical interpretation of verbosity levels used by these helpers is:
 * - Level 0: user-facing summary information (e.g., selected thread count, random engine and seed)
 * - Level 1: high-level initialization steps (reserved for broader startup narratives)
 * - Level 2: detailed tracing (e.g., logging each UI command as it is applied)
 *
 * Debug output (when enabled by the logger implementation) is expected to include
 * low-level diagnostic details useful for troubleshooting configuration issues,
 * such as expanded command sequences and option-derived decisions.
 *
 * \n\n
 * \author \n &copy; Maurizio Ungaro
 * \author e-mail: ungaro@jlab.org
 * \n\n\n
 */
