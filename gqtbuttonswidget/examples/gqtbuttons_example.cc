/**
 * \file gqtbuttons_example.cc
 * \brief Minimal GUI example for the Qt Buttons Widgets module.
 *
 * \ingroup gqtbuttons_examples
 *
 * \details
 * This example demonstrates:
 * - Parsing basic options using GOptions
 * - Conditionally starting a Qt event loop only when \c --gui is enabled
 * - Creating a \c GQTButtonsWidget using Qt resource-based SVG icons
 * - Exiting the application after a user-configurable timeout
 */

/**
 * \defgroup gqtbuttons_examples Qt Buttons Widgets examples
 * \brief Example programs demonstrating how to use the Qt Buttons Widgets module.
 *
 * \details
 * Each example is a standalone program showing typical integration patterns.
 * The module mainpage links to this group and provides short excerpts.
 */

#include "gQtButtonsWidget_options.h"
#include "gQtButtonsWidget.h"

// qt
#include <QApplication>
#include <QMainWindow>
#include <QTimer>

/**
 * \brief Example entry point.
 *
 * \details
 * Behavior:
 * - Creates a GOptions instance to parse command-line arguments.
 * - Reads:
 *   - \c gui : whether to run with a Qt GUI.
 *   - \c tt  : timeout (milliseconds) after which the GUI exits automatically.
 * - If \c gui is true:
 *   - Creates \c QApplication
 *   - Instantiates \c GQTButtonsWidget with two icon base names
 *   - Shows the widget
 *   - Uses \c QTimer::singleShot() to quit after \c tt milliseconds
 * - If \c gui is false:
 *   - Runs in CLI mode (no GUI loop)
 *
 * \param argc Standard argument count.
 * \param argv Standard argument vector.
 * \return \c EXIT_SUCCESS on clean completion.
 */
int main(int argc, char* argv[]) {
	// Initialize options and logging.
	// Note: "hello" is the example's program name used by GOptions.
	auto gopts = std::make_shared<GOptions>(argc, argv, GOptions("hello"));

	// Example switches/scalars expected to exist in the calling environment.
	// If not provided, GOptions will use its internal defaults.
	auto gui     = gopts->getSwitch("gui");
	auto timeout = gopts->getScalarDouble("tt");
	int  ret     = EXIT_SUCCESS;

	// Create a QApplication only when GUI mode is enabled.
	QApplication* app = nullptr;
	if (gui) {
		app = new QApplication(argc, argv);
	}

	// Icon base names:
	// The resource path must match the path used in the module's .qrc file.
	std::vector<std::string> bicons;
	bicons.emplace_back(":/images/firstButton");
	bicons.emplace_back(":/images/secondButton");

	if (gui) {
		// Create and show the icon-based buttons widget.
		GQTButtonsWidget window(128, 128, bicons);
		window.show();

		// Quit after the requested timeout (milliseconds).
		QTimer::singleShot(timeout, [] {
			QCoreApplication::quit(); // stops the event loop
		});

		ret = QApplication::exec();

		// Clean up GUI resources.
		delete app;
	}
	else {
		// CLI mode:
		// This example intentionally does not execute any non-GUI behavior.
	}

	return ret;
}
