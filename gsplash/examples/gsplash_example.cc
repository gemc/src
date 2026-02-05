/**
 * @file gsplash_example.cc
 * @ingroup gsplash_examples
 * @brief Example program demonstrating GSplash usage in GUI and CLI modes.
 *
 * \anchor gsplash_example
 *
 * This example shows how to:
 * - Build a shared options object (GOptions) and a logger (GLogger).
 * - Conditionally enable GUI mode (via \c --gui).
 * - Create a GSplash instance using \ref GSplash::create "create()".
 * - Post immediate and delayed messages using \ref GSplash::message "message()" and
 *   \ref GSplash::messageAfter "messageAfter()".
 *
 * Usage notes:
 * - When \c --gui is enabled, the program starts a \c QApplication event loop and displays a window.
 * - The splash is closed automatically when the application quits (after a timeout).
 * - When \c --gui is disabled, the example runs in command-line mode and does not create GUI objects.
 */

// gsplash
#include "gsplash.h"

// qt
#include <QApplication>
#include <QMainWindow>

/**
 * @brief Entry point for the GSplash example.
 *
 * The program demonstrates two execution modes:
 * - GUI mode: enabled by the \c --gui switch, creates a \c QApplication, shows a main window, and displays a splash.
 * - CLI mode: default when \c --gui is not enabled, skips all GUI initialization.
 *
 * The timeout value is read from the \c tt scalar option and used to exit the GUI event loop.
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line arguments.
 * @return EXIT_SUCCESS on normal completion.
 */
int main(int argc, char* argv[]) {
	// Initialize options and logging for the module.
	auto gopts   = std::make_shared<GOptions>(argc, argv, gsplash::defineOptions());
	auto log     = std::make_shared<GLogger>(gopts, SFUNCTION_NAME, GSPLASH_LOGGER);

	// Read runtime configuration:
	// - gui controls whether we instantiate QApplication and show the splash.
	// - tt controls how long the example remains active in GUI mode.
	auto gui     = gopts->getSwitch("gui");
	auto timeout = gopts->getScalarDouble("tt");

	int  ret     = EXIT_SUCCESS;

	log->info(0, "Starting gsplash example...");

	// Optional GUI setup (only if --gui is passed).
	QApplication* app    = nullptr;
	QMainWindow*  window = nullptr;

	if (gui) {
		log->info(0, "g4dialog", "Running in GUI mode...");

		// Create the Qt application and a main window to demonstrate returning focus.
		app    = new QApplication(argc, argv);
		window = new QMainWindow();
		window->setWindowTitle(QString::fromUtf8("displayUI example"));

		// Create the splash screen and display messages.
		// The factory returns nullptr when GUI is disabled; here we are in GUI mode by construction.
		auto gsplash = GSplash::create(gopts, "example.png");
		if (gsplash) {
			gsplash->message("Some text I want to show");
			gsplash->messageAfter(500, "Some other text I want to show");
		}

		window->show();

		// Quit after timeout. Notice the additional delay needed for the messageAfter.
		QTimer::singleShot(timeout + 500, [] {
			QCoreApplication::quit(); // Stop the event loop.
		});

		ret = QApplication::exec();

		// Clean up heap-allocated Qt objects.
		delete window;
		delete app;

	}
	else {
		// CLI mode (no QApplication is created).
		log->info(0, "Running gsplash in command line mode...");
	}

	return ret;
}
