#pragma once

// qt
#include <QCoreApplication>

// geant4
#include "G4MTRunManager.hh"
#include "G4UImanager.hh"

// cpp
#include <memory>

// gemc
#include "glogger.h"
#include "gsplash.h"
#include "gbatch_session.h"

namespace gemc {

/**
 * @file gemcUtilities.h
 * @brief Utility helpers for runtime setup, command preparation, and random-engine initialization.
 *
 * This header contains small, self-contained utilities used during GEMC startup:
 * - Qt application instantiation for GUI and batch modes
 * - Computation of the number of threads to use
 * - Preparation of Geant4 UI command sequences
 * - Execution of those commands through the UI manager
 * - Random engine selection and seeding
 */

/**
 * @defgroup gemc_utilities_module GEMC Utilities Module
 * @brief Runtime utilities used during GEMC initialization and configuration.
 *
 * This module is focused on **startup/runtime glue code**:
 * - determining thread counts from user options
 * - producing Geant4 UI command lists based on options
 * - applying UI command lists
 * - selecting and seeding the random engine
 * - creating a Qt application instance in GUI or batch mode
 *
 * Ownership and lifecycle considerations are documented per-function.
 * @{
 */

/**
 * @brief Create a Qt application instance suitable for GUI or batch execution.
 *
 * - When @p gui is `true`, this function returns a `QApplication` instance
 *   (returned as a `std::unique_ptr<QCoreApplication>` for uniform handling).
 * - When @p gui is `false`, this function:
 *   1. Creates a `GBatch_Session` instance.
 *   2. Transfers its ownership to Geant4 by calling
 *      `G4UImanager::GetUIpointer()->SetCoutDestination(...)`.
 *   3. Returns a `QCoreApplication` instance.
 *
 * @par Ownership and lifecycle
 * - The returned `std::unique_ptr<QCoreApplication>` owns the Qt application object.
 * - In batch mode, the created `GBatch_Session` is **released** and then owned
 *   by Geant4 via `SetCoutDestination`. The caller must not delete it.
 *
 * @param argc Standard `main()` argument count (passed by reference as required by Qt).
 * @param argv Standard `main()` argument array (passed through to Qt).
 * @param gui If `true`, create a `QApplication`; otherwise create a `QCoreApplication`
 *            and redirect Geant4 cout to a `GBatch_Session`.
 *
 * @return A `std::unique_ptr<QCoreApplication>` owning either a `QCoreApplication`
 *         or a `QApplication` instance.
 */
inline std::unique_ptr<QCoreApplication>
makeQtApplication(int& argc, char* argv[], bool gui) {

	if (!gui) {
		auto gbatch = std::make_unique<GBatch_Session>();
		G4UImanager::GetUIpointer()->SetCoutDestination(gbatch.release()); // release ownership to Geant4

		return std::make_unique<QCoreApplication>(argc, argv);
	}
	return std::make_unique<QApplication>(argc, argv);
}

/**
 * @brief Determine the number of worker threads to use for the run.
 *
 * This helper reads the `nthreads` option from @p gopts and clamps it to the
 * available CPU core count reported by `G4Threading::G4GetNumberOfCores()`.
 *
 * Behavior:
 * - If `nthreads == 0`, the maximum number of available cores is used.
 * - If `nthreads > available_cores`, the value is clamped to `available_cores`.
 * - Otherwise the user-provided `nthreads` value is used.
 *
 * The function logs the chosen number of threads using @p log.
 *
 * @param gopts Runtime options provider. The key used is `nthreads`.
 * @param log Logger used to report the chosen thread count.
 *
 * @return The number of threads that should be used.
 */
int get_nthreads(const std::shared_ptr<GOptions>& gopts, const std::shared_ptr<GLogger>& log);

/**
 * @brief Build a list of Geant4 UI commands that reduce verbosity across subsystems.
 *
 * The returned commands are intended to keep Geant4 output quiet in typical runs.
 * They include `.../verbose 0` style settings for major categories such as:
 * control, hit, process, tracking, geometry navigator, event, cuts, run, material,
 * visualization, and particle messages.
 *
 * @param gopts Runtime options provider (currently unused by this implementation).
 * @param log Logger (currently unused by this implementation).
 *
 * @return A list of UI command strings to be executed in order.
 */
std::vector<std::string> verbosity_commands(const std::shared_ptr<GOptions>& gopts,
                                           const std::shared_ptr<GLogger>& log);

/**
 * @brief Build a list of Geant4 UI commands needed at startup.
 *
 * This function prepares commands based on options:
 * - `check_overlaps` (typically provided by the G4 system options set)
 * - `gui`
 *
 * Behavior:
 * - If `check_overlaps == 2`, schedule an overlap test with a resolution of 50 points.
 * - If `check_overlaps >= 100`, schedule an overlap test with that many points.
 * - Always include `"/run/initialize"` after overlap configuration.
 * - If `gui` is `false`, return the list after initialization.
 * - If `gui` is `true`, append visualization scene/viewer configuration commands:
 *   drawing the volume, configuring trajectories/hits, background color,
 *   disabling/enabling auto-refresh around setup, and finally flushing the viewer.
 *
 * @param gopts Runtime options provider. Keys used include `check_overlaps` and `gui`.
 * @param log Logger used to report which overlap test (if any) will be executed.
 *
 * @return A list of UI command strings to be executed in order.
 */
std::vector<std::string> initial_commands(const std::shared_ptr<GOptions>& gopts,
                                          const std::shared_ptr<GLogger>& log);

/**
 * @brief Execute a sequence of Geant4 UI commands through the UI manager.
 *
 * This function obtains the global UI manager pointer and applies each command
 * in @p commands in order. Each command is logged at an informational verbosity
 * suitable for tracing configuration activity.
 *
 * @param gopts Runtime options provider (currently unused by this implementation).
 * @param log Logger used to trace command execution.
 * @param commands List of UI command strings to apply, in order.
 */
void run_manager_commands(const std::shared_ptr<GOptions>& gopts,
                          const std::shared_ptr<GLogger>& log,
                          const std::vector<std::string>& commands);

/**
 * @brief Select and start the random engine, then seed it.
 *
 * This function reads:
 * - `randomEngine` : the name of the random engine implementation
 * - `seed` : the numeric seed value
 *
 * If the seed is `SEEDNOTSET`, a seed is generated from a combination of wall time,
 * process CPU clock, and the process id.
 *
 * @par Supported engine names
 * The engine names correspond to CLHEP random engines (examples include
 * `DRand48Engine`, `DualRand`, `HepJamesRandom`, `MTwistEngine`, `RanluxEngine`, etc.).
 * The selection is performed by matching the `randomEngine` option string.
 *
 * @par Error handling
 * If an unknown engine name is provided, an error is logged using the module error code
 * `EC__RANDOMENGINENOTFOUND`.
 *
 * @param gopts Runtime options provider. Keys used: `randomEngine`, `seed`.
 * @param log Logger used to report the chosen engine and seed, or errors.
 */
void start_random_engine(const std::shared_ptr<GOptions>& gopts, const std::shared_ptr<GLogger>& log);

/** @} */ // end of group gemc_utilities_module

} // namespace gemc
