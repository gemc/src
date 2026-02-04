/**
 * \mainpage GEMC Detector System Module (gsystem)
 *
 * \tableofcontents
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
 * @section verbosity_sec Verbosity and logging
 *
 * Most classes in this module derive from GBase and therefore use a GLogger.
 * Typical verbosity behavior:
 * - Level 0: high-level milestones and key warnings (e.g. missing optional files).
 * - Level 1: normal informational messages (e.g. which systems/factories are loading).
 * - Level 2: detailed diagnostics (e.g. expanded SQL queries, per-column dumps).
 * - Debug: very fine-grained tracing such as constructor/function entry markers.
 *
 * The exact interpretation is logger-dependent, but the module is structured so that higher
 * levels add detail without changing semantics.
 *
 * @section examples_sec Examples
 *
 * @subsection example_gsystem Example: gsystem_example.cc
 * A minimal program that constructs GOptions using gsystem option definitions and then
 * constructs a GWorld, triggering full system load (factories, volumes/materials, modifiers).
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
