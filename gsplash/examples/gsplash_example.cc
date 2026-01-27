// gsplash
#include "gsplash.h"

// qt
#include <QApplication>
#include <QMainWindow>

int main(int argc, char* argv[]) {
	// Initialize options and logging
	auto gopts   = std::make_shared<GOptions>(argc, argv, gsplash::defineOptions());
	auto log     = std::make_shared<GLogger>(gopts, SFUNCTION_NAME, GSPLASH_LOGGER);
	auto gui     = gopts->getSwitch("gui");
	auto timeout = gopts->getScalarDouble("tt");
	int  ret     = EXIT_SUCCESS;

	log->info(0, "Starting gsplash example...");

	// Optional GUI setup (only if --gui is passed)
	QApplication* app    = nullptr;
	QMainWindow*  window = nullptr;

	if (gui) {
		log->info(0, "g4dialog", "Running in GUI mode...");
		app    = new QApplication(argc, argv);
		window = new QMainWindow();
		window->setWindowTitle(QString::fromUtf8("displayUI example"));

		// create the splash screen and display messages
		auto gsplash = GSplash::create(gopts, "example.png");
		gsplash->message("Some text I want to show");
		gsplash->messageAfter(500, "Some other text I want to show");

		window->show();

		// quit after timeout. Notice the additional delay needed for the messageAfter
		QTimer::singleShot(timeout + 500, [] {
			QCoreApplication::quit(); // stop the event loop
		});

		ret = QApplication::exec();

		delete window;
		delete app;

	}
	else {
		// CLI mode
		log->info(0, "Running gsplash in command line mode...");
	}

	return ret;
}
