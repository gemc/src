/**
 * \mainpage GDetector
 *
 * @section intro_sec Introduction
 *
 * The gdetector module provides the Geant4 detector-construction bridge for GEMC:
 * it builds a Geant4 geometry from GEMC world definitions and then installs sensitive
 * detectors, EM fields, and digitization routines.
 *
 * The primary entry point is \ref GDetectorConstruction, which implements the Geant4
 * detector-construction hooks and connects them to the GEMC geometry/digitization model.
 *
 * @section topics_sec Topics
 *
 * @defgroup gdetector_module gdetector module (geometry + SD/field + digitization bridge)
 * @brief Core APIs and implementation for building geometry and installing SD/field and digitization.
 *
 * @defgroup gdetector_examples gdetector examples
 * @brief Runnable examples showing typical module usage patterns.
 *
 * @ingroup gdetector_module
 *
 * @section ownership_sec Ownership and lifecycle
 *
 * Ownership is split between GEMC-managed objects and Geant4-managed runtime state:
 * - GEMC world objects (e.g. GWorld and related system/volume structures) are owned by
 *   \ref GDetectorConstruction and are recreated when geometry is rebuilt.
 * - Geant4 geometry stores (solids/logical/physical volumes) are managed by Geant4 and are
 *   explicitly cleaned before rebuild to prevent stale state.
 * - Sensitive detectors are created and registered so that Geant4 can invoke them during tracking.
 * - Field infrastructure is installed per volume; a thread-local field container is used to match
 *   Geant4 multi-threaded execution patterns.
 *
 * Typical lifecycle:
 * - Construct options (via gdetector::defineOptions()).
 * - Instantiate \ref GDetectorConstruction with the options.
 * - Build geometry (Geant4 calls \c Construct()).
 * - Install SD/fields (Geant4 calls \c ConstructSDandField()).
 * - Optionally reload geometry with \ref GDetectorConstruction::reload_geometry "reload_geometry()".
 *
 * @section architecture_sec Architecture
 *
 * Design notes:
 * - The detector builder constructs a GEMC world first, then translates it into Geant4 constructs
 *   via the G4World adapter.
 * - Sensitive detectors are created by digitization name, allowing multiple logical volumes to map
 *   to a single GSensitiveDetector instance.
 * - Digitization routines are loaded after sensitive detectors exist, then bound to each detector.
 * - EM fields are configured per volume using a named field definition and installed via per-volume
 *   field managers.
 *
 * @section options_sec Available Options and usage
 *
 * The gdetector module aggregates options from multiple dependent modules by calling
 * gdetector::defineOptions(). The current aggregation includes:
 * - gsystem option set
 * - g4system option set
 * - gdynamicdigitization option set
 * - gsensitivedetector option set
 *
 * Usage pattern:
 * - Construct a GOptions instance passing the aggregated definition function:
 *   \code{.cpp}
 *   auto gopts = std::make_shared<GOptions>(argc, argv, gdetector::defineOptions());
 *   \endcode
 *
 * If additional options are added by the module itself in the future, they should be listed in
 * this section and documented at their point of definition.
 *
 * @section verbosity_sec Module verbosity
 *
 * The module uses the logger name \c "gdetector" (see GDETECTOR_LOGGER).
 * Typical verbosity behavior:
 * - Level 0: high-level run summaries (e.g. geometry tallies, major configuration outcomes).
 * - Level 1: key configuration steps (e.g. digitization routine creation and successful definitions).
 * - Level 2: verbose per-volume/per-detector setup details (e.g. SD assignments, field attachment).
 * - Debug output: detailed diagnostic traces for construction paths and plugin setup decisions.
 *
 * @section examples_sec Examples
 *
 * The following example demonstrates a minimal usage path including geometry reload and a
 * multi-threaded digitization loop.
 *
 * - \ref gdetector_example : Multi-threaded digitization example (gdetector_example.cc)
 *
 * Example snippet:
 * \code{.cpp}
 * auto gopts = std::make_shared<GOptions>(argc, argv, gdetector::defineOptions());
 * auto gdetector = std::make_shared<GDetectorConstruction>(gopts);
 * auto gsystems = gsystem::getSystems(gopts);
 * gdetector->reload_geometry(gsystems);
 * \endcode
 *
 * \n\n
 * \author \n &copy; Maurizio Ungaro
 * \author e-mail: ungaro@jlab.org
 * \n\n\n
 */
