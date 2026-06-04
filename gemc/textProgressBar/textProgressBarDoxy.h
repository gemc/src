
/**
 * \mainpage textProgressBar module
 *
 * This module provides a lightweight, dependency-minimal text progress bar for terminal output.
 * It is designed for long-running loops where periodically printing a single-line progress indicator
 * improves usability without introducing GUI or logging dependencies.
 *
 * \section tp_overview Overview
 * The core API is the TextProgressBar class:
 * - Construct the progress bar with a title, a width (in characters), and a value range.
 * - Call TextProgressBar::setProgress "setProgress()" with the current progress value.
 *
 * The progress bar is rendered on a single console line using a carriage return (`\r`) so that repeated
 * updates overwrite the same line. When progress reaches the last step, a newline is printed.
 *
 * \section tp_features Features
 * - Configurable bar width and title text.
 * - Configurable characters for bar delimiters and fill/advance indicators.
 * - Throttling via an internal step size so the console is not flooded with output.
 *
 * \section tp_examples Examples
 * The module includes the following example(s):
 *
 * \subsection tp_ex_bar examples/bar.cc
 * Demonstrates basic usage in a tight loop: construct a TextProgressBar, then call
 * TextProgressBar::setProgress "setProgress()" as the loop index advances.
 *
 * \code
 * int nevents = 100000000;
 * TextProgressBar bar(30, "  index progress: ", 1, nevents);
 * for (int i = 1; i < nevents; i++) {
 *   bar.setProgress(i);
 * }
 * \endcode
 *
 * \section tp_ownership Ownership and scope
 * This module is intentionally small and self-contained:
 * - Public API is defined in textProgressBar.h.
 * - Behavior is implemented in textProgressBar.cc.
 * - Module-wide constants are defined in textProgressBarConventions.h.
 *
 * \section tp_verbosity Verbosity
 * This module does not implement a logger interface and does not define verbosity levels.
 * Output is produced only by explicitly calling TextProgressBar::setProgress "setProgress()".
 */
