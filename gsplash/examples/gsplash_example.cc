// gsplash
#include "gsplash.h"

// qt
#include <QApplication>
#include <QMainWindow>

int main(int argc, char* argv[]) {

	auto gopts = std::make_shared<GOptions>(argc, argv, gsplash::defineOptions());
	auto log   = std::make_shared<GLogger>(gopts, SFUNCTION_NAME, GSPLASH_LOGGER);

	bool gui = gopts->getSwitch("gui");

	QScopedPointer<QCoreApplication> gApp(new QApplication(argc, argv));

	auto gsplash = GSplash::create(gopts, "example.png");

	if (gui) {
		// create the splash screen
		gsplash->message("Some text I want to show");

		auto window = new QMainWindow; // own it with Qt’s parent system
		window->show();

		/* ---- quit after 0.5s ---- */
		// need to move gsplash here because the capture is trying to copy it
		/* ---- quit after 0.5s ---- */
		QTimer::singleShot(
			500, window,
			[g = std::move(gsplash), window]() mutable   // <-- move + mutable
			{
				if (g) g->finish(window);                // still safely owned here
				QCoreApplication::quit();
			});

		return QApplication::exec();
	}

	return EXIT_SUCCESS;
}
