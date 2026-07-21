/**
 * \mainpage GDynamicDigitization Module
 *
 * \tableofcontents
 *
 * \image html gdynamicdigitization-flow.svg "Digitization plugin lifecycle" width=900px
 *
 * \section gdynamicdigitization_intro Introduction
 * The GDynamicDigitization module defines the base interface and utilities used to
 * implement detector digitization routines as dynamically loadable plugins.
 *
 * At a high level, a digitization plugin is responsible for:
 * - defining readout timing/storage rules via
 *   \ref GDynamicDigitization::defineReadoutSpecs "defineReadoutSpecs()"
 * - optionally loading constants and a translation table via
 *   \ref GDynamicDigitization::loadConstants "loadConstants()" and
 *   \ref GDynamicDigitization::loadTT "loadTT()"
 * - converting a true hit into digitized output via \ref GDynamicDigitization::digitizeHit "digitizeHit()"
 * - optionally collecting a standardized “true information” payload via
 *   \ref GDynamicDigitization::collectTrueInformation "collectTrueInformation()"
 *
 * \image html digitization-workflow.svg "Sensitive-detector and digitization data flow" width=900px
 *
 * \section gdynamicdigitization_components Key components
 * - GDynamicDigitization : Abstract base class defining the plugin surface.
 * - GTouchableModifiers : Helper container used when a digitizer needs to compute
 *   weighted/weighted-time modifiers for touchables.
 * - GReadoutSpecs : Small immutable specification used to compute electronics time bin indices.
 *
 * \section gdynamicdigitization_options Configuration
 * \ref gdynamicdigitization::defineOptions "gdynamicdigitization::defineOptions()" defines:
 * - \c recordZeroEdep: retain hits with exactly zero deposited energy.
 * - \c digitization_variation: override the geometry variation used to load constants and translation tables.
 * - \c applyThresholds: comma- or space-separated digitization systems that apply channel thresholds, or
 *   \c all.
 * - \c applyInefficiencies: systems that apply per-channel efficiency rejection, or \c all.
 *
 * It also aggregates the event-data, run-data, translation-table, and plugin-factory option sets required by
 * the digitization workflow.
 *
 * \section gdynamicdigitization_verbosity Verbosity and logging
 * Most classes in this module log through the project logging infrastructure.
 * Typical meanings (by convention) are:
 * - level 0 : essential information and high-level workflow milestones (e.g. which plugin was loaded).
 * - level 1 : configuration details and one-time initialization information (e.g. readout specs).
 * - level 2 : per-hit or per-event informational tracing (may be verbose for large jobs).
 * - debug : fine-grained, developer-oriented messages intended for diagnosing behavior
 *   (often includes intermediate indices, computed values, or branch decisions).
 *
 * \section gdynamicdigitization_examples Examples
 * The module ships example programs illustrating how to implement and load plugins:
 *
 * \subsection gdynamicdigitization_example_plugin Example plugin implementation
 * - File : examples/gplugin_test_example.h, examples/gplugin_test_example.cc
 * - Summary : Implements a minimal digitization plugin (readout specs, constants, TT, hit digitization)
 *   and exposes the required factory symbol \c GDynamicDigitizationFactory.
 *
 * \subsection gdynamicdigitization_example_loader Example plugin loading and multi-threaded usage
 * - File : examples/plugin_load_example.cc
 * - Summary : Loads a plugin with gdynamicdigitization::dynamicRoutinesMap(), calls
 *   \ref GDynamicDigitization::loadConstants "loadConstants()", then runs a small multi-threaded
 *   event loop that digitizes a few hits per event.
 *
 * \n\n
 * \author \n &copy; Maurizio Ungaro
 * \author e-mail: ungaro@jlab.org
 * \n\n\n
 */
