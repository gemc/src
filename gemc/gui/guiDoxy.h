/**
 * \mainpage GEMC GUI Module
 *
 * \tableofcontents
 *
 * \image html gui-flow.svg "Top-level GEMC GUI coordination" width=900px
 *
 * \image html display-gui.svg "GEMC graphical interface" width=900px
 *
 * @section gui_intro Introduction
 *
 * This module provides the Qt-based graphical user interface for GEMC. The primary entry point is the
 * GemcGUI widget, which builds the application layout and connects UI actions to the EventDispenser
 * so the user can run events interactively and switch between different visualization/configuration pages.
 *
 * The GUI is a composition of a left-side button bar (page selector), a right-side stacked content area
 * (pages such as display, dialog, setup, and geometry tree), and a bottom board widget for status/controls.
 *
 *
 * @section gui_ownership Ownership and lifecycle
 *
 * - GemcGUI is a \c QWidget and participates in Qt parent/child ownership.
 * - Most Qt widgets created with \c this as parent are owned and destroyed by Qt automatically.
 * - A subset of widgets are explicitly deleted in ~GemcGUI(); this matches the current implementation.
 *   When extending the GUI, prefer a single ownership model (Qt parent ownership or explicit deletion),
 *   and keep it consistent for new members.
 *
 *
 * @section gui_architecture Architecture
 *
 * @subsection gui_design_notes Design notes
 *
 * - **Layout composition**
 *   - Left pane: a GQTButtonsWidget provides a button list and emits selection changes.
 *   - Right pane: a \c QStackedWidget hosts multiple “pages” and is driven by the left pane selection.
 *   - Top row: run controls (run, cycle, stop) and counters (number of events, current event number).
 *   - Bottom: a GBoard widget is attached below the main layout.
 *
 * - **Event execution**
 *   - The GUI delegates event execution to EventDispenser.
 *   - “Run” executes a batch and then updates the displayed event number.
 *   - “Cycle” runs one event periodically using a \c QTimer; “Stop” stops the timer.
 *
 * - **Page switching**
 *   - The left button widget emits a selection-change signal.
 *   - A private slot \c change_page reads the selected index and applies it to the
 *     \c QStackedWidget, updating the visible page.
 *
 *
 * @section gui_options Available Options and usage
 *
 * This module does not define a standalone GUI option schema. Instead it consumes:
 * - Options carried by GOptions and passed into child pages such as the display and dialog widgets.
 * - Run control values managed by EventDispenser:
 *   - The "N. Events" field updates the number of events via EventDispenser::setNumberOfEvents().
 *   - The initial value is read from EventDispenser::getTotalNumberOfEvents().
 *
 * @section gui_verbosity Module verbosity
 *
 * The top-level widget has no module-specific logger. Child pages use their own logger categories, including
 * \c g4display, \c g4dialog, \c gtree, and \c dbselect.
 *
 *
 * \author &copy; Maurizio Ungaro
 * \author e-mail: ungaro@jlab.org
 */


/**
 * @defgroup gemc_gui_module GEMC GUI Module
 *
 * \brief Qt user interface widgets that integrate with the GEMC run control and visualization pages.
 *
 * This group collects the GUI-facing classes and helpers that assemble the main window and connect
 * user actions to the event-processing backend.
 */
