// gsplash
#include "gsplash.h"

// qt
#include <QApplication>

int main(int argc, char* argv[]) {
	auto gopts = new GOptions(argc, argv, gsplash::defineOptions());
	auto log   = std::make_shared<GLogger>(gopts, GSPLASH_LOGGER, "gpslash_example");

	bool gui = gopts->getSwitch("gui");

	QScopedPointer<QCoreApplication> gApp(new QApplication(argc, argv));

	GSplash* gsplash = nullptr;
	Q_INIT_RESOURCE(qt_resources_example);

	if (gui) {
		gsplash = new GSplash(log, "example.png");

		QTime dieTime = QTime::currentTime().addSecs(1);
		gsplash->message("Some text I want to show");
		gApp->processEvents();

		QMainWindow window;
		window.show();

		// wait 1 second
		while (QTime::currentTime() < dieTime) { QCoreApplication::processEvents(QEventLoop::AllEvents, 100); }
		gsplash->finish(&window);

		return gApp->exec();
	}

	return EXIT_SUCCESS;
}
