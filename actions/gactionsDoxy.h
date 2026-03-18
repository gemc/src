/**
 * @file gactionsDoxy.h
 * @brief Doxygen entry point for the GEMC actions module.
 *
 * This file defines the module-level Doxygen group and the main documentation page
 * for the action subsystem responsible for Geant4 action registration and execution.
 */

/**
 * @defgroup gactions_module GEMC actions module
 * @brief Geant4 action initialization and per-thread action components for GEMC.
 *
 * This module contains the classes that register and execute the Geant4 user actions
 * used by GEMC during a run. It centralizes the action lifecycle, threading model,
 * and the interaction between generation, event processing, run accumulation,
 * digitization, and data streaming.
 */

/**
 * @mainpage GEMC actions module
 *
 * @section gactions_intro Introduction
 *
 * The GEMC actions module is the bridge between the Geant4 action framework and
 * the GEMC event-processing pipeline. Its purpose is to register the Geant4 user
 * actions required by a simulation run and to connect those actions to GEMC-specific
 * services such as particle generation, hit digitization, event data collection,
 * run data accumulation, and streamer publication.
 *
 * In practical terms, this module provides:
 * - GAction, the action-initialization entry point used by the run manager.
 * - GPrimaryGeneratorAction, which creates event primaries.
 * - GRunAction, which manages run begin/end hooks and creates the per-thread run object.
 * - GEventAction, which processes hit collections at the end of each event.
 * - GRun, the thread-local run object created for each active run.
 *
 * @section gactions_ownership Ownership and lifecycle
 *
 * The ownership model follows the Geant4 action system:
 * - The Geant4 run manager owns the action-initialization object and the actions
 *   registered through it.
 * - In sequential mode, a single set of actions is created and used for the whole run.
 * - In multithreaded mode, worker threads receive their own action objects, while
 *   the master thread typically owns only the actions that are meaningful at
 *   master scope.
 *
 * For this module:
 * - GAction stores shared configuration and the shared digitization-routine map.
 * - GPrimaryGeneratorAction is created on worker threads and in sequential mode.
 * - GEventAction is created on worker threads and in sequential mode.
 * - GRunAction is created both on worker threads and on the master thread.
 * - GRun is created per thread through
 *   GRunAction::GenerateRun "GenerateRun()".
 *
 * The data ownership inside the module is intentionally split:
 * - Shared configuration is passed as a shared pointer so all actions read the same
 *   runtime settings.
 * - The digitization-routine map is shared because it represents configuration and
 *   plugin-provided behavior reused across the run.
 * - Event data and run data are created locally during processing and then published
 *   or merged according to their collection mode.
 *
 * @section gactions_architecture Architecture
 *
 * @subsection gactions_design_notes Design notes
 *
 * The module is organized around the Geant4 user-action lifecycle.
 *
 * Geant4 expects an action-initialization object derived from \c G4VUserActionInitialization.
 * GEMC provides that role through GAction. During initialization, GAction registers:
 * - GPrimaryGeneratorAction on worker threads and in sequential mode.
 * - GEventAction on worker threads and in sequential mode.
 * - GRunAction on worker threads, in sequential mode, and on the master thread.
 *
 * The run and event flow is applied here as follows:
 *
 * 1. **Action registration**
 *    - \ref GAction::Build "Build()" is called for worker threads and for sequential execution.
 *      It registers the primary generator action, the run action, and the event action.
 *    - \ref GAction::BuildForMaster  "BuildForMaster()" is called for the master thread.
 *      It registers only the run action, because the master does not generate primaries
 *      or execute per-event callbacks.
 *
 * 2. **Run creation**
 *    - At the start of a Geant4 run, \c GRunAction::GenerateRun   creates
 *      a GRun instance for the current thread.
 *    - This gives each active thread its own run container and access to the shared
 *      digitization-routine map for the lifetime of that run.
 *
 * 3. **Begin of run**
 *    - GRunAction::BeginOfRunAction() allocates the run-level
 *      bookkeeping objects used by the current thread.
 *    - It also inspects the registered digitization routines to determine whether the
 *      run needs event-mode streamers, run-mode streamers, or both.
 *    - Worker threads lazily create and open thread streamers when event-mode publication
 *      is needed.
 *    - The master thread lazily creates and opens run streamers when run-mode publication
 *      is needed.
 *
 * 4. **Primary generation**
 *    - For each event on a worker thread, GPrimaryGeneratorAction configures a \c G4ParticleGun
 *      using the configured Gparticle objects and injects the primaries into the event.
 *
 * 5. **End of event**
 *    - \ref GEventAction::EndOfEventAction "EndOfEventAction()" reads the hit collections
 *      produced by sensitive detectors.
 *    - For each hit collection, it resolves the matching digitization routine from the
 *      collection name.
 *    - Event-mode digitizers add digitized and truth payload to the event container and
 *      publish it through the thread streamers.
 *    - Run-mode digitizers append their payload to the thread-local run data instead of
 *      publishing immediately.
 *
 * 6. **End of run**
 *    - Worker-thread \c GRunAction::EndOfRunAction  closes thread
 *      streamers and moves completed run data into a protected process-wide pool.
 *    - Master-thread \c GRunAction::EndOfRunAction  collects those
 *      worker results, merges them, publishes merged run data through the run streamers,
 *      and closes the master-side run streamers.
 *
 * This design keeps event-level publication local to worker threads while centralizing
 * run-level publication on the master thread.
 *
 * @section gactions_options Available Options and their usage
 *
 * The module exposes a single aggregation helper through gaction::defineOptions().
 * That helper combines the options defined by:
 * - geventaction::defineOptions()
 * - grunaction::defineOptions()
 * - gprimaryaction::defineOptions()
 * - grun::defineOptions()
 *
 * In the provided implementation, these module-local helpers currently return logger-scoped
 * option containers without introducing dedicated action-specific switches of their own.
 *
 * Usage:
 * - Call gaction::defineOptions() during application or module setup.
 * - Merge the returned GOptions object into the application's full option set.
 * - Construct GAction with the shared GOptions instance and the shared
 *   digitization-routine map.
 *
 * Even though the action module itself does not currently define additional user-facing
 * switches in these helpers, the primary-generation path depends on particle definitions
 * loaded through the generator support code in gparticle_options.h. Those particle-related
 * options must therefore also be configured for the full action pipeline to operate as expected.
 *
 * @section gactions_verbosity Module verbosity
 *
 * The classes in this module derive from the common GEMC base/logger infrastructure,
 * so they follow the standard module verbosity behavior.
 *
 * Typical output expectations are:
 * - **Level 0** prints high-level lifecycle information such as major run boundaries
 *   and the creation of essential processing components.
 * - **Level 1** prints operational information useful for normal monitoring, including
 *   fallback behavior and important state transitions such as streamer creation.
 * - **Level 2** prints detailed processing traces, such as collection-by-collection
 *   event handling and streamer open/close activity.
 * - **Debug** prints developer-oriented diagnostics, including constructor traces,
 *   function entry traces, and thread identifiers.
 *
 * @author Maurizio Ungaro
 * @author e-mail: ungaro@jlab.org
 */