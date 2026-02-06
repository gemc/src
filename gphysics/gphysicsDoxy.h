#pragma once

/**
 * @defgroup gphysics_module gphysics module : Geant4 physics list selection and construction
 *
 * @brief Module responsible for selecting and instantiating a Geant4 physics list from user options.
 *
 * This module provides a small integration layer between GEMC options (via GOptions) and
 * Geant4 physics list factories. It enables users to select a reference physics list and to
 * extend it with additional physics constructors using the same syntax supported by the
 * Geant4 extensible factory.
 */

/**
 * @mainpage gphysics module
 *
 * @section gphysics_intro Introduction
 * The gphysics module is a lightweight wrapper that builds and exposes a Geant4 physics list
 * based on runtime options. Its primary responsibility is:
 * - Parse the user-selected physics list string (and optional extensions).
 * - Instantiate the corresponding reference list through the Geant4 extensible factory.
 * - Register a small set of additional constructors required by GEMC defaults (for example a step limiter).
 *
 * The produced physics list is returned as a raw pointer to \c G4VModularPhysicsList, which is then
 * owned and managed by the Geant4 run manager / application lifecycle.
 *
 * @section gphysics_ownership Ownership and lifecycle
 * The module allocates the physics list through the Geant4 factory and returns it via
 * GPhysics::getPhysList(). The underlying object is expected to be consumed by the calling
 * application and managed according to the Geant4 lifecycle (typically the run manager owns
 * and deletes the physics list at shutdown).
 *
 * GPhysics itself is a small helper object used to coordinate option parsing, logging,
 * and factory invocation.
 *
 * @section gphysics_arch Architecture
 * @subsection gphysics_design Architecture overview
 * The module is composed of:
 * - **Options definition**: gphysics::defineOptions() declares the user-facing options and switch.
 * - **Builder**: the GPhysics class reads the options and asks the Geant4 extensible factory for the
 *   requested reference physics list.
 * - **Diagnostics**: a switch can print available lists and constructors and then exit early.
 *
 * @subsection gphysics_notes Design notes
 * - The physics list string may be extended by the user with separators supported by the Geant4 extensible
 *   factory (for example adding constructors using \c '+' or replacing components using \c '_').
 * - This module deliberately avoids exposing Geant4 factory details in the public API: consumers only need
 *   a pointer to \c G4VModularPhysicsList.
 *
 * @section gphysics_verbosity Module verbosity
 * This module uses the logger name \c gphysics. When the module verbosity is:
 * - **0**: only essential messages are printed (for example version banners and required listings).
 * - **1**: additional informational messages are printed (for example selection summaries).
 * - **2**: more verbose informational messages are printed (for example detailed load confirmations).
 *
 * When debug is enabled, the module prints constructor-level diagnostics (for example when the GPhysics
 * object is instantiated).
 *
 * @section options_sec Available Options and their usage
 *
 * This module reads the following option keys from the runtime option provider:
 *
 * - `phys_list`
 *   - Type: string
 *   - Meaning: Geant4 reference physics list selection, with optional EM replacement and optional extra constructors
 *   - Behavior:
 *     - base list is selected by name (e.g. `FTFP_BERT`)
 *     - EM replacement may be appended via an underscore suffix (e.g. `_LIV`)
 *     - additional physics constructors may be appended using `+` (e.g. `+G4OpticalPhysics`)
 *   - Note: the accepted tokens depend on the Geant4 version and the registered factory inventory.
 *
 * - `showPhysics`
 *   - Type: boolean (switch)
 *   - Meaning: print available physics lists and constructors and exit
 *   - Behavior:
 *     - when enabled, the module prints the curated list and terminates early
 *
 * @section gphysics_examples Examples
 * - \ref gphysics_example_main "gphysics_example" : Minimal example that defines options and instantiates GPhysics.
 *
 * @subsection gphysics_example_snippet Example snippet
 * @code
 * auto gopts = std::make_shared<GOptions>(argc, argv, gphysics::defineOptions());
 * auto gphysics = std::make_shared<GPhysics>(gopts);
 * @endcode
 */
