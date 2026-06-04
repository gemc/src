/**
 * \mainpage gboard module
 *
 * \tableofcontents
 *
 * ## Overview
 *
 * The **gboard** module provides a small Qt-based log viewer widget (GBoard) and a Geant4 UI session adapter
 * (GUI_Session) that forwards Geant4 output into that widget.
 *
 * The primary intent is to support GUI applications where you want Geant4 text output to appear inside
 * a dedicated panel rather than a terminal.
 *
 * ### Key components
 * - GBoard: a QWidget containing:
 *   - a filter/search field
 *   - a clear button
 *   - a save button
 *   - a read-only rich-text log area
 * - GUI_Session: a \c G4UIsession implementation that:
 *   - receives Geant4 cout/cerr
 *   - splits text into lines
 *   - converts common ANSI SGR sequences to HTML
 *   - appends the result to GBoard
 *
 * @section options_sec Available Options and their usage
 *
 * This module currently does not define or consume any module-specific option keys.
 *
 * Notes:
 * - Host applications commonly control whether the GUI is enabled through the global `gui` switch
 *   defined by \ref GOptions::GOptions "GOptions(argc,argv,...)".
 * - Logger routing and verbosity for this module are typically controlled by the global `verbosity`
 *   and `debug` structured options (also defined by \ref GOptions::GOptions "GOptions(argc,argv,...)" ),
 *   using the logger name \c gboard (see \c GBOARD_LOGGER).
 *
 * ## Ownership and lifecycle
 *
 * - GBoard is a Qt widget: it owns its child widgets via Qt parent/child ownership.
 * - GUI_Session **does not own** the GBoard pointer it receives. The GUI (or caller) must ensure
 *   the board outlives the session, and the session detaches itself from \c G4UImanager at destruction.
 *
 * ## Verbosity
 *
 * This module uses the standard GEMC logging conventions via classes derived from glogger.
 * Typical meaning (implementation-specific but consistent in practice):
 * - **Level 0**: high-level lifecycle messages and essential user-facing information.
 * - **Level 1**: detailed but still routine operational messages (initialization details, user actions).
 * - **Level 2**: verbose operational traces (frequent state updates, fine-grained progress).
 * - **Debug**: developer-oriented diagnostics, usually including extra context useful when troubleshooting.
 *
 * Adjust verbosity using the module logger name \c gboard (see \c GBOARD_LOGGER).
 *
 * ## Examples
 *
 * - **gboard_example.cc**: Demonstrates how to embed GBoard in a \c QMainWindow and route Geant4 output
 *   to it using GUI_Session. The example supports both GUI and CLI modes.
 *
 * ## Module ownership
 *
 * The module is owned and maintained as part of the GEMC codebase. Contributions should follow the
 * project contribution guidelines and preserve the documented threading/ownership contracts described here.
 */
