/**
 * \mainpage GEMC G4Display Module Documentation
 *
 * \tableofcontents
 *
 * \image html g4display-flow.svg "Interactive visualization control flow" width=900px
 *
 * \section g4display_intro Introduction
 *
 * The G4Display module provides a Qt-based graphical user interface layer for controlling and interacting with
 * Geant4 visualization from within GEMC. It is designed to expose common viewer controls (camera direction,
 * lighting, scene style toggles, cutaway planes, etc.) through a set of QWidget-based panels.
 *
 * The module assumes a running Geant4 UI session and communicates with it by sending visualization commands
 * to \c G4UImanager. These commands are the same as would be used interactively in a Geant4 macro, but are
 * generated in response to GUI events.
 *
 * \image html display-gui.svg "GEMC graphical interface with the Display page selected" width=900px
 *
 * \image html display-utilities.svg "GEMC scene decoration and annotation controls" width=900px
 *
 * \section g4display_structure Module structure
 *
 * The module is primarily composed of:
 * - \ref G4Display : main widget that hosts visualization control tabs.
 * - \ref G4DisplayView : tab that provides camera, lighting, slice controls, and view toggles.
 * - \ref G4DisplayUtilities : tab with scene decoration and text controls.
 * - \ref G4SceneProperties : helper that generates Geant4 visualization commands for initial scene setup.
 * - g4display options helpers (see \ref g4display::defineOptions and related getters) used to map GOptions
 *   configuration into simple structs consumed by the GUI and scene initializers.
 *
 *
 *
 * \section g4display_verbosity Verbosity and logging
 *
 * Several classes in this module are built on top of GEMC base utilities that provide structured logging via
 * GLogger.
 * The logger name for this module is typically \c "g4display" (see \c G4DISPLAY_LOGGER).
 *
 * The conventional meaning of the verbosity levels is:
 * - Level 0 : high-level user-facing messages (major actions and state changes).
 * - Level 1 : additional informational messages (configuration choices, derived parameters).
 * - Level 2 : detailed informational messages (frequent GUI-driven updates, fine-grained state).
 * - Debug   : developer diagnostics (construction/destruction traces, command strings, internal decisions).
 *
 *
 * \section g4display_usage Typical usage
 *
 * In a GEMC application, the GUI widget \ref G4Display is instantiated with a shared GOptions instance and
 * embedded into a parent window. Users interact with the GUI controls; the module translates those interactions
 * into Geant4 visualization commands sent via \c G4UImanager.
 *
 * The helper \ref G4SceneProperties can be used during initialization to build a consistent default scene and
 * viewer configuration from options.
 *
 * \section g4display_options Configuration
 *
 * \ref g4display::defineOptions "g4display::defineOptions()" provides the following option groups:
 * - \c g4view: driver, window dimensions and position, curve precision, background, and cloud density.
 * - \c g4camera: initial camera theta and phi.
 * - \c g4light: initial light-source theta and phi.
 * - \c g4decoration: scale, axes, event ID, date, logos, and frame settings.
 * - \c g4text: optional scene annotations.
 * - \c dawn and \c useDawn: DAWN output view and enable switch.
 *
 * The typed accessors \ref g4display::getG4View "getG4View()",
 * \ref g4display::getG4Camera "getG4Camera()", \ref g4display::getG4Light "getG4Light()", and
 * \ref g4display::getG4Decorations "getG4Decorations()" project those option nodes into the structures used by
 * the GUI and scene initializer.
 *
 *
 * \section g4display_build Building and dependencies
 *
 * This module depends on:
 * - \c Qt6 (Widgets module)
 * - \c Geant4 (UI and visualization)
 * - GEMC core utilities (GOptions, GBase, GLogger, etc.)
 *
 *
 * \section g4display_examples Examples
 *
 * The following example demonstrates how to create a minimal Geant4 visualization environment and optionally
 * attach the G4Display GUI.
 *
 * - g4display_example_cc : Minimal G4Display GUI usage
 *   \n Summary: create GOptions + logger, initialize \c G4VisExecutive, and build \ref G4Display inside a Qt
 *   window.
 *
 *
 * \author \n &copy; Maurizio Ungaro
 * \author e-mail: ungaro@jlab.org
 */
