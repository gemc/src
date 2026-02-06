/**
 *
 * \defgroup gemc_gsystem gsystem
 * \brief GEMC detector-system module (gsystem): geometry + materials loading and world assembly.
 *
 * \defgroup gemc_gsystem_core Core types
 * \ingroup gemc_gsystem
 * \brief Core data types: GWorld, GSystem, GVolume, GMaterial, GModifier.
 *
 * \defgroup gemc_gsystem_modifiers Modifiers
 * \ingroup gemc_gsystem
 * \brief Post-load volume modifications (shift/tilt/existence).
 *
 * \defgroup gemc_gsystem_factories Factory
 * \ingroup gemc_gsystem
 * \brief System loaders (sqlite / ascii / CAD / GDML) and factory infrastructure.
 *
 * \defgroup gemc_gsystem_factories_sqlite SQLite factory
 * \ingroup gemc_gsystem_factories
 * \brief sqlite-backed loader implementation.
 *
 * \defgroup gemc_gsystem_factories_text Text factory
 * \ingroup gemc_gsystem_factories
 * \brief ASCII text loader implementation.
 *
 * \defgroup gemc_gsystem_factories_cad CAD factory
 * \ingroup gemc_gsystem_factories
 * \brief CAD (STL directory) loader implementation.
 *
 * \defgroup gemc_gsystem_factories_gdml GDML factory
 * \ingroup gemc_gsystem_factories
 * \brief GDML loader placeholder.
 *
 * \defgroup gemc_gsystem_examples Examples
 * \ingroup gemc_gsystem
 * \brief Small example programs for the module.
 *
 * \mainpage GEMC Detector System Module (gsystem)
 *
 * @section intro_sec Introduction
 *
 * The gsystem module provides the infrastructure that turns detector configuration into
 * an in-memory representation of geometry and materials.
 *
 * Core concepts:
 * - A GSystem is a logical detector subsystem (e.g. calorimeter, tracker).
 * - A GVolume is one placed volume record belonging to a GSystem.
 * - A GMaterial is a material definition belonging to a GSystem.
 * - A GWorld assembles multiple systems, applies modifiers, and assigns final names.
 * - A GSystemFactory loads a system from a concrete source (sqlite, ASCII, CAD, ...).
 *
 * @section ownership_sec Ownership and lifecycle
 *
 * - The world owns the system map container, and stores systems as shared pointers.
 * - Each system owns its collections of volumes and materials.
 * - Factories are temporary objects used during load; they may keep transient state such as
 *   open DB handles or search paths and should release them in closeSystem().
 *
 * @section factories_sec Factories
 *
 * Factory selection is driven by system configuration:
 * - \c sqlite : loads geometry/materials from a sqlite database.
 * - \c ascii  : loads geometry/materials from text files with positional fields.
 * - \c CAD    : imports volumes from CAD assets (e.g. STL).
 * - \c GDML   : placeholder for future GDML support.
 *
 * @section options_sec Available Options and their usage
 *
 * This module reads the following option keys from the runtime option provider:
 *
 * - `gsystem`
 *   - Type: sequence of maps
 *   - Meaning: list of detector subsystems to load and assemble into the world
 *   - Behavior:
 *     - each entry becomes one \c GSystem created by \c gsystem::getSystems()
 *     - the selected factory controls the source backend (sqlite / ascii / CAD / GDML placeholder)
 *   - Subkeys used by this module:
 *     - `name` (string, mandatory): system name (and/or source identifier)
 *     - `factory` (string): factory label (default: sqlite)
 *     - `variation` (string): geometry variation (default: \c "default")
 *     - `annotations` (string): optional system annotations (default: \c UNINITIALIZEDSTRINGQUANTITY)
 *
 * - `gmodifier`
 *   - Type: sequence of maps
 *   - Meaning: post-load volume modification rules applied by the world
 *   - Behavior:
 *     - each entry becomes one \c GModifier created by \c gsystem::getModifiers()
 *   - Subkeys used by this module:
 *     - `name` (string): volume name (default: \c goptions::NODFLT sentinel)
 *     - `shift` (string): shift triple (default: \c GSYSTEMNOMODIFIER)
 *     - `tilt` (string): tilt triple (default: \c GSYSTEMNOMODIFIER)
 *     - `isPresent` (bool): if \c false, remove volume from world (default: \c true)
 *
 * - `sql`
 *   - Type: string
 *   - Meaning: sqlite file path (or SQL host, depending on factory) used as the default geometry source
 *   - Behavior:
 *     - used as the base database/location when creating systems
 *     - overridden by `ascii_db` when \c factory == \c "ascii"
 *
 * - `ascii_db`
 *   - Type: string
 *   - Meaning: alternative search path/root used by the ASCII factory
 *   - Behavior:
 *     - when a system entry selects the ASCII factory, this value replaces `sql` as the source root
 *
 * - `experiment`
 *   - Type: string
 *   - Meaning: experiment selection shared by all systems
 *   - Behavior:
 *     - passed into each created \c GSystem as the experiment context
 *
 * - `runno`
 *   - Type: integer
 *   - Meaning: conditions/run number shared by all systems
 *   - Behavior:
 *     - passed into each created \c GSystem as the run context used for variation/run-dependent selection
 *
 * Note: this module’s option schema is composed by \c gsystem::defineOptions(), and it also aggregates
 * options from \c gfactory::defineOptions(). Any additional plugin-loader options are documented there.
 *
 * @section verbosity_sec Verbosity and logging
 *
 * Most classes in this module derive from GBase and therefore use a GLogger.
 *
 * Typical verbosity behavior (convention for classes derived from GBase / using GLogger):
 * - Level 0: high-level milestones and key warnings (e.g. missing optional files, major fallbacks).
 * - Level 1: normal informational messages (e.g. which systems/factories are loading).
 * - Level 2: detailed diagnostics (e.g. expanded SQL queries, per-column dumps, per-volume summaries).
 * - Debug: very fine-grained tracing such as constructor/function entry markers, internal state snapshots.
 *
 * The exact interpretation is logger-dependent, but the module is structured so that higher
 * levels add detail without changing semantics.
 *
 * @section examples_sec Examples
 *
 * @subsection example_gsystem Example : gsystem_example.cc
 * Minimal program that:
 * - builds a GOptions instance with \c gsystem::defineOptions();
 * - constructs a GWorld, triggering full system load (factories, volumes/materials, modifiers).
 *
 * See the example's full documentation here: \ref gsystem_example_anchor "gsystem_example.cc".
 *
 * Example snippet:
 * \code
 * ./gsystem_example -gsystem="[{name: b1, factory: sqlite, variation: default}]" -sql=gemc.db
 * \endcode
 *
 * @section notes_sec Design notes
 *
 * - The module uses serialized parameter vectors to represent DB/ASCII rows. Parsing is positional.
 * - Post-load modifiers (shift/tilt/existence) are applied by the world after all volumes exist.
 * - Final Geant4 names are assigned as \c "<system>/<volume>" during world bookkeeping.
 *
 * \n\n
 * \author \n &copy; Maurizio Ungaro
 * \author e-mail: ungaro@jlab.org
 * \n\n\n
 */
