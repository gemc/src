/**
 * \file gQtButtonsWidgetDoxy.h
 * \brief Module-level documentation for the Qt Buttons Widgets module.
 *
 * \mainpage Qt Buttons Widgets Module
 *
 * \section intro_sec Introduction
 * This module provides lightweight Qt widgets to present **button-like UI controls** in a compact form:
 * - \c GQTButtonsWidget : an icon-based button strip implemented with a \c QListWidget in icon mode.
 * - \c GQTToggleButtonWidget : a group of checkable \c QPushButton objects (toggle buttons).
 *
 * These widgets are intended for **GUI-thread usage** inside Qt applications.
 *
 * \section design_sec Design overview
 * \subsection icon_buttons_subsec Icon buttons (\c GQTButtonsWidget)
 * The icon-buttons widget uses a \c QListWidget configured in \c QListView::IconMode. Each logical button
 * is represented by a \c QListWidgetItem whose icon is updated based on interaction:
 * - State \c 1 : "normal" icon
 * - State \c 2 : "pressed" icon
 *
 * Icons are loaded by filename convention:
 * \code
 * <base_icon_name>_<state>.svg
 * \endcode
 * For example, if the base icon is \c ":/images/firstButton" then the widget will look for:
 * \code
 * :/images/firstButton_1.svg
 * :/images/firstButton_2.svg
 * \endcode
 *
 * \subsection toggle_buttons_subsec Toggle buttons (\c GQTToggleButtonWidget)
 * The toggle-buttons widget creates one \c QPushButton per title string, marks it checkable, and arranges
 * buttons either vertically (\c QVBoxLayout) or horizontally (\c QHBoxLayout). The widget tracks the last
 * pressed button index and emits a signal whenever that index changes.
 *
 * @section options_sec Available Options and their usage
 *
 * This module currently does not define or consume any module-specific option keys.
 *
 * Notes:
 * - Host applications typically decide GUI creation and logging via global keys defined by
 *   \ref GOptions::GOptions "GOptions(argc,argv,...)" (for example `gui`, `verbosity`, and `debug`),
 *   but these widgets themselves do not read options directly.
 *
 * \section usage_sec Usage
 * The typical usage pattern is:
 * - Construct the widget with the desired dimensions and labels/icons.
 * - Connect to signals (e.g. \c buttonPressedIndexChanged()) to react to user interaction.
 * - Optionally invoke helper functions (e.g. \c press_button(), \c reset_buttons()) to drive state from code.
 *
 * \section examples_sec Examples
 * The project contains example code demonstrating how to instantiate and display these widgets.
 * Each entry includes a short summary and a small code excerpt.
 *
 * - \ref gqtbuttons_examples "Qt Buttons Widgets examples"
 *   - **gqtbuttons_example.cc**: minimal demo that creates a \c GQTButtonsWidget, shows it, and exits after
 *     a configurable timeout.
 *     \code
 *     std::vector<std::string> bicons = {":/images/firstButton", ":/images/secondButton"};
 *     GQTButtonsWidget window(128, 128, bicons);
 *     window.show();
 *     \endcode
 *
 * \author \n &copy; Maurizio Ungaro
 * \author e-mail: ungaro@jlab.org
 */
