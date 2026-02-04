/**
 * \mainpage GEMC G4Display Module Documentation
 *
 * \section intro_sec Introduction
 *
 * The G4Display module provides a Qt-based graphical user interface layer for controlling and interacting with
 * Geant4 visualization from within GEMC. It is designed to expose common viewer controls (camera direction,
 * lighting, scene style toggles, cutaway planes, etc.) through a set of QWidget-based panels.
 *
 * The module assumes a running Geant4 UI session and communicates with it by sending visualization commands
 * to \c G4UImanager. These commands are the same as would be used interactively in a Geant4 macro, but are
 * generated in response to GUI events.
 *
 *
 * \section structure_sec Module structure
 *
 * The module is primarily composed of:
 * - \ref G4Display : main widget that hosts visualization control tabs.
 * - \ref G4DisplayView : tab that provides camera, lighting, slice controls, and view toggles.
 * - \ref G4DisplayUtilities : tab intended for auxiliary display utilities (currently a minimal placeholder).
 * - \ref G4SceneProperties : helper that generates Geant4 visualization commands for initial scene setup.
 * - g4display options helpers (see \ref g4display::defineOptions and related getters) used to map GOptions
 *   configuration into simple structs consumed by the GUI and scene initializers.
 *
 *
 *
 * \section verbosity_sec Verbosity and logging
 *
 * Several classes in this module are built on top of GEMC base utilities that provide structured logging via Glogger.
 * The logger name for this module is typically \c "g4display" (see \c G4DISPLAY_LOGGER).
 *
 * The conventional meaning of the verbosity levels is:
 * - Level 0 : high-level user-facing messages (major actions and state changes).
 * - Level 1 : additional informational messages (configuration choices, derived parameters).
 * - Level 2 : detailed informational messages (frequent GUI-driven updates, fine-grained state).
 * - Debug   : developer diagnostics (construction/destruction traces, command strings, internal decisions).
 *
 *
 * \section usage_sec Typical usage
 *
 * In a GEMC application, the GUI widget \ref G4Display is instantiated with a shared GOptions instance and embedded
 * into a parent window. Users interact with the GUI controls; the module translates those interactions into Geant4
 * visualization commands sent via \c G4UImanager.
 *
 * The helper \ref G4SceneProperties can be used during initialization to build a consistent default scene and viewer
 * configuration from options.
 *
 *
 * \section build_sec Building and dependencies
 *
 * This module depends on:
 * - \c Qt6 (Widgets module)
 * - \c Geant4 (UI and visualization)
 * - GEMC core utilities (GOptions, GBase, Glogger, etc.)
 *
 *
 * \section examples_sec Examples
 *
 * The following example demonstrates how to create a minimal Geant4 visualization environment and optionally
 * attach the G4Display GUI.
 *
 * - g4display_example_cc : Minimal G4Display GUI usage
 *   \n Summary: create GOptions + logger, initialize \c G4VisExecutive, build \ref G4Display inside a Qt window.
 *
 *
 * \author \n &copy; Maurizio Ungaro
 * \author e-mail: ungaro@jlab.org
 */
