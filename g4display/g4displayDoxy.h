/**
 * \mainpage GEMC G4Display Module Documentation
 *
 * \section intro_sec Introduction
 *
 * This documentation describes the G4Display module, a component of the GEMC (Geant4 Monte Carlo) software framework.
 * The G4Display module provides a Qt-based graphical user interface for controlling and interacting with
 * Geant4's visualization system. It allows users to manipulate camera angles, lighting, display styles (wireframe, solid),
 * apply cutaway planes (slices), visualize magnetic fields, and access other visualization utilities.
 *
 * \section structure_sec Module Structure
 *
 * The module is primarily composed of the following classes:
 * - G4Display: The main QTabWidget that hosts different control panels.
 * - G4DisplayView: A QWidget tab providing controls for camera, lighting, slicing, and view styles.
 * - G4DisplayUtilities: A placeholder QWidget tab for future utility functions.
 * - GLogger: A utility class for structured and verbose logging throughout the application.
 * - g4display_options: A namespace and associated functions for defining and parsing command-line options related to visualization settings (viewer type, camera position, etc.).
 *
 * \section usage_sec Usage
 *
 * The G4Display widget is typically instantiated within the main GEMC application. It takes a `GOptions` object to configure its initial state based on command-line arguments or configuration files.
 * The user interacts with the tabs to send commands directly to the Geant4 `G4UImanager`.
 *
 * \section build_sec Building and Dependencies
 *
 * This module depends on:
 * - Qt5/Qt6 (Widgets module)
 * - Geant4 (UI and Visualization libraries)
 * - GEMC Core libraries (GOptions, GLogger, etc.)
 *
 * Refer to the main GEMC build instructions for details.
 *
 * \section improve_sec Future Improvements
 *
 * - Implement functionality in the G4DisplayUtilities tab.
 * - Explore using Geant4 visualization commands instead of direct OpenGL calls for antialiasing if possible.
 * - Enhance error handling for invalid user input (e.g., non-numeric slice values).
 * - Consider model-view patterns for managing G4 state more robustly.
 *
 * \n\n
 * \author \n &copy; Maurizio Ungaro
 * \author e-mail: ungaro@jlab.org
 * \n\n\n
 */