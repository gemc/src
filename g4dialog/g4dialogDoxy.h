/**
 * \mainpage G4Dialog
 *
 * \tableofcontents
 *
 * \section g4dialog_intro Introduction
 *
 * G4Dialog is a lightweight Qt-based user interface module that provides a convenient front-end
 * for browsing and executing \c Geant4 UI commands. It is designed to be embedded as a widget
 * inside a Qt application, and it can also be exercised from a standalone example program.
 *
 * The module currently includes:
 * - A tabbed widget container (G4Dialog)
 * - A "Geant4 Commands" tab (G4Commands) featuring:
 *   - A searchable command tree
 *   - Contextual help for the selected command
 *   - A command prompt and history recall
 *
 * \section g4dialog_features Key features
 * - **Command discovery**: browse the \c Geant4 command tree and quickly locate commands by filtering.
 * - **Contextual help**: view guidance and parameter descriptions for the selected command.
 * - **Command execution**: paste a selected command into the prompt and execute it.
 * - **History**: avoid retyping by recalling previously executed commands.
 *
 * \section g4dialog_verbosity Verbosity and logging
 *
 * This module follows the GEMC-style logging conventions via GBase/Glogger integration.
 * The logger name for this module is \c G4DIALOG_LOGGER ("g4dialog").
 *
 * Typical verbosity expectations:
 * - **info level 0**: minimal user-facing lifecycle messages (startup, major mode changes).
 * - **info level 1**: additional operational hints (e.g., which UI path is used, configuration echoes).
 * - **info level 2**: verbose operational tracing suitable for diagnosing configuration issues.
 * - **debug**: detailed diagnostics useful when developing the UI (selection changes, command execution flow,
 *   widget-level interactions).
 *
 * The exact verbosity switches are provided by the surrounding framework (e.g., GOptions).
 *
 * \section g4dialog_examples Examples
 *
 * The following example(s) are part of this module documentation:
 *
 * \subsection g4dialog_example1 Example: Standalone widget usage
 *
 * **Summary**: builds a minimal \c Geant4 visualization manager and runs G4Dialog either in GUI mode
 * (Qt event loop) or in a command-line-only mode.
 *
 * **Link**: \ref g4dialog_example_main "g4dialog_example.cc documentation"
 *
 * **Snippet**:
 * \code
 * auto gopts = std::make_shared<GOptions>(argc, argv, g4dialog::defineOptions());
 * auto *g4dialog = new G4Dialog(gopts, window);
 * window->setCentralWidget(g4dialog);
 * \endcode
 *
 * \author
 * \n &copy; Maurizio Ungaro
 * \n e-mail: ungaro@jlab.org
 */
