// splash
#include "gsplash.h"
#include <QElapsedTimer>
#include <QThread>
#include <algorithm>
using std::string;

// Factory creation:
// - Returns nullptr when GUI is disabled (headless runs).
// - Otherwise constructs GSplash and attempts to load the requested image.
std::unique_ptr<GSplash> GSplash::create(
	const std::shared_ptr<GOptions>& gopts,
	const string&                    img,
	double                           splashTime) {
	if (!gopts || !gopts->getSwitch("gui"))
		return nullptr; // head-less run → no splash
	return std::unique_ptr<GSplash>(new GSplash(gopts, img, splashTime));
}

// Constructor summary:
// - Resolves the image either from GSPLASH env var (special token) or from a provided name/path.
// - Tries filesystem path first, then Qt resource lookup.
// - Creates and shows QSplashScreen only when a valid pixmap is available.
GSplash::GSplash(const std::shared_ptr<GOptions>& gopts, const string& imageName, double splashTime)
	: GBase(gopts, GSPLASH_LOGGER) {
	QPixmap pixmap;
	const auto option_splash_time = gopts->getScalarDouble(GSPLASH_TIME_OPTION);
	splash_time = option_splash_time >= 0.0 ? option_splash_time : splashTime;

	// If no explicit image is selected, load it from the GSPLASH environment variable.
	if (imageName == NOSPLASHIMAGESELECTED) {
		if (const char* filename = std::getenv(GSPLASHENVIRONMENT); filename) {
			pixmap.load(filename); // loads or leaves null
		}
		else {
			log->error(ERR_NOSPLASHENVFOUND,
			           "Environment variable ", GSPLASHENVIRONMENT,
			           " must point to an image file.");
		}
	}
	else {
		// Try filesystem path first (e.g. "example.png"), then Qt resource (":/example.png")
		if (!pixmap.load(QString::fromStdString(imageName))) {
			pixmap.load(QString(":/%1").arg(QString::fromStdString(imageName)));
		}

		if (pixmap.isNull())
			log->error(ERR_NOSPLASHENVFOUND, "Image ", imageName, " not found.");
	}

	// Create the splash only when we have a valid pixmap; otherwise leave it inactive.
	if (!pixmap.isNull()) {
		splash = std::make_unique<QSplashScreen>(pixmap);
		splash->show();
		QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
	}
}


void GSplash::finish(QWidget* callingWindow) const {
	if (!splash) return;

	const int splash_time_ms = static_cast<int>(std::max(0.0, splash_time) * 1000.0);
	if (splash_time_ms > 0) {
		QElapsedTimer timer;
		timer.start();
		while (timer.elapsed() < splash_time_ms) {
			QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
			QThread::msleep(10);
		}
	}

	splash->finish(callingWindow);
}


void GSplash::messageAfter(int delay_ms, const std::string& msg) {
	if (!splash) return;

	// Guard against the splash being destroyed before the timer fires.
	QPointer<QSplashScreen> sp = splash.get();
	QTimer::singleShot(delay_ms, sp, [sp, qmsg = QString::fromStdString(msg)] {
		if (!sp) return;
		sp->showMessage(qmsg, Qt::AlignLeft, Qt::black);
		QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
	});
}


void GSplash::message(const std::string& msg) {
	if (!splash) return;

	splash->showMessage(QString::fromStdString(msg), Qt::AlignLeft, Qt::black);
	QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
}
