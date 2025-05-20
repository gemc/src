// gsplash
#include "gsplash.h"

// qt
#include <QApplication>
#include <QMainWindow>

int main(int argc, char* argv[]) {
	auto gopts = new GOptions(argc, argv, gsplash::defineOptions());
	auto log   = std::make_shared<GLogger>(gopts, GSPLASH_LOGGER, "gsplash_example");

	bool gui = gopts->getSwitch("gui");

	QScopedPointer<QCoreApplication> gApp(new QApplication(argc, argv));

	if (gui) {
		// create the splash screen
		auto gsplash = new GSplash(log, "example.png"); // will delete itself later
		gsplash->message("Some text I want to show");

		auto window = new QMainWindow; // own it with Qt’s parent system
		window->show();

		/* ---- quit after 0.5s ---- */
		QTimer::singleShot(500, [gsplash, window] {
			gsplash->finish(window);
			delete gsplash;                 // or gsplash->deleteLater() if GSplash inherits QObject
			QCoreApplication::quit();       // no down‑cast, no warning
		});
		return QApplication::exec();
	}

	return EXIT_SUCCESS;
}
