/**
 * @file gactionsDoxy.h
 * @brief Doxygen entry point for the GActions module documentation.
 *
 * This file provides the module mainpage and defines the Doxygen group used by
 * the action classes in this module.
 */

/**
 * @defgroup gactions_module GActions module
 * @brief Geant4 action initialization and per-thread action components.
 *
 * This group contains the classes that wire together Geant4 user actions
 * (primary generation, run actions, event actions) and expose a single place
 * to aggregate the options used by these actions.
 */

/**
 * \mainpage GActions
 *
 * @section intro_sec Introduction
 *
 * The GActions module is responsible for configuring and instantiating the
 * Geant4 user actions used by GEMC. It provides:
 *
 * - A single action-initialization entry point (GAction) that registers the
 *   worker and master actions.
 * - A primary generator action (GPrimaryGeneratorAction) that produces the
 *   event primaries using a configured set of Gparticle objects.
 * - A run action (GRunAction) that creates the per-thread GRun object and
 *   configures per-thread streaming connections.
 * - An event action (GEventAction) that collects hits, digitizes them, and
 *   publishes the resulting event data to the configured streamers.
 *
 * @section ownership_sec Ownership and lifecycle
 *
 * - The top-level Geant4 framework owns the user action objects registered via
 *   the Geant4 action-initialization mechanism.
 * - Each worker thread instantiates its own set of actions; these objects are
 *   thread-local by design.
 * - The master thread typically instantiates only the run action.
 * - The digitization routines map is treated as a shared, read-mostly structure
 *   that is provided at construction time and reused by worker actions.
 *
 * @section architecture_sec Architecture
 *
 * @subsection design_notes Design notes
 *
 * - **Registration:** GAction registers the actions via the Geant4 action
 *   initialization API (\c G4VUserActionInitialization).
 * - **Per-thread state:** GRunAction creates a per-thread streamer map on demand
 *   and (re)opens connections at the start of each run for worker threads.
 * - **Event processing:** GEventAction iterates over hit collections for the
 *   event, digitizes each hit via the routine associated with the collection
 *   name, and publishes the event data through all configured streamers.
 * - **Run object:** GRun is the run container created per thread by
 *   GRunAction::GenerateRun().
 *
 * @section options_sec Available Options and usage
 *
 * The module aggregates options from the sub-components:
 *
 * - event action options (from geventaction::defineOptions())
 * - run action options (from grunaction::defineOptions())
 * - primary generator options (from gprimaryaction::defineOptions())
 * - run container options (from grun::defineOptions())
 *
 * Usage pattern:
 *
 * - Call gaction::defineOptions() to obtain the complete option set used by the
 *   actions subsystem.
 * - Merge it into the global configuration, then construct GAction with the
 *   shared GOptions pointer.
 *
 * @section verbosity_sec Module verbosity
 *
 * The classes in this module use the standard GEMC logging conventions through
 * the common logging infrastructure.
 *
 * - Verbosity level **0** typically prints high-level run lifecycle information
 *   (e.g., run boundaries and major initialization steps).
 * - Verbosity level **1** prints additional operational information such as
 *   warnings and non-fatal issues.
 * - Verbosity level **2** prints detailed per-run/per-event traces (for example,
 *   collection names processed, or per-particle information during generation).
 * - Debug output prints developer-oriented traces such as constructor activity,
 *   function entry, and thread identifiers.
 *
 * \n\n
 * \author \n &copy; Maurizio Ungaro
 * \author e-mail: ungaro@jlab.org
 * \n\n\n
 */
