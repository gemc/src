#pragma once
// Make sure that the next string after subsubsection or subsection is indexed correctly

/**
 * @file gsdDoxy.h
 * @ingroup gsd_module
 * @brief Doxygen documentation entry point for the GSensitiveDetector module.
 */

/**
 * @defgroup gsd_module gsd : GSensitiveDetector module
 *
 * @brief Sensitive-detector glue between Geant4 hit processing and GEMC digitization plugins.
 *
 * This group documents the module implemented by GSensitiveDetector and its integration
 * with dynamic digitization routines.
 */

/**
 * \mainpage GSensitiveDetector module
 *
 * \section gsd_intro Introduction
 *
 * The GSensitiveDetector module provides the thread-local sensitive detector implementation used by GEMC to
 * translate Geant4 step information into GHit objects stored in a Geant4 hits collection.
 *
 * Conceptually, this module is the bridge between:
 * - the Geant4 sensitive-detector callback interface (\c G4VSensitiveDetector), and
 * - GEMC digitization plugins (GDynamicDigitization) that decide whether to process a step, how to interpret
 *   a touchable, and what hit information (HitBitSet) is stored.
 *
 * The high-level event flow is:
 * - **Event initialization**: allocate and register the hits collection, reset per-event caches, and load hit specs.
 * - **Step processing**: optionally skip steps, build one or more touchables, and create or update hits accordingly.
 * - **End of event**: finalize any per-event bookkeeping (the actual storage is handled by the Geant4 event).
 *
 *
 * \section gsd_ownership Ownership and lifecycle
 *
 * - **Threading model**: GSensitiveDetector instances are expected to be thread-local, as indicated by inline
 *   comments and usage patterns.
 * - **Digitization routine**: the module holds a shared pointer to a GDynamicDigitization instance that must be
 *   assigned via GSensitiveDetector::assign_digi_routine() before processing begins.
 * - **Touchable registry**: a registry of volume-name to GTouchable mappings is populated during detector construction
 *   (e.g., by GDetectorConstruction). After registration, this module treats the registry as authoritative for the run.
 * - **Event-owned hit collection**: the Geant4 hits collection is created at Initialize() and registered into the event.
 *   The event lifecycle determines when the hits collection is reclaimed.
 *
 *
 * \section gsd_arch Architecture
 *
 * \subsection gsd_design Architecture and design notes
 *
 * **Key responsibilities**
 * - Determine the HitBitSet once per event using the digitization plugin readout specifications.
 * - Maintain a per-event list of already-seen touchables (touchableVector) to decide whether a step creates a new hit
 *   or updates an existing one.
 * - Store hits in a \c G4THitsCollection<GHit> (typedef GHitsCollection).
 *
 * **Processing model**
 * - For each step, the plugin can return **one or more** processed touchables.
 *   This supports cases such as segmentation or step splitting where a single Geant4 step contributes to multiple
 *   logical detector elements.
 * - For each processed touchable:
 *   - assign the track id for later identification and grouping,
 *   - decide if it is new for this event,
 *   - either create a new GHit or locate the existing GHit and append step information.
 *
 *
 * \section gsd_options Available options and usage
 *
 * This module defines its options via gsensitivedetector::defineOptions().
 *
 * - If the returned GOptions is empty, then no module-specific options are currently exposed.
 * - If GOptions::defineOptions() contributes applicable options to this module from elsewhere, those options should be
 *   listed here (as part of the broader project configuration).
 *
 * Current behavior:
 * - gsensitivedetector::defineOptions() returns a GOptions instance initialized with the logger name used by this module.
 *
 *
 * \section gsd_verbosity Module verbosity
 *
 * This module logs through the logger name \c "gsd".
 *
 * Typical verbosity meaning:
 * - **level 0**: critical errors only (e.g., missing hit collection or missing touchable mappings).
 * - **level 1**: event-level messages (e.g., start/end of event hooks).
 * - **level 2**: detailed step and touchable processing (e.g., how many processed touchables were produced, new vs existing).
 * - **debug**: constructor-level diagnostics and very verbose internal traces.
 *
 *
 * \section gsd_ci Continuous Integration
 *
 * This module is built and tested as part of the overall GEMC CI pipeline.
 *
 * \n\n
 * \author \n &copy; Maurizio Ungaro
 * \author e-mail ungaro@jlab.org
 * \n
 */
