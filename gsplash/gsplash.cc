// splash
#include "gsplash.h"
using std::string;

std::unique_ptr<GSplash> GSplash::create(
	const std::shared_ptr<GOptions>& gopts,
	const string&                    img) {
	if (!gopts || !gopts->getSwitch("gui"))
		return nullptr; // head‑less run → no splash
	return std::unique_ptr<GSplash>(new GSplash(gopts, img));
}

GSplash::GSplash(const std::shared_ptr<GOptions>& gopts, const string& imageName)
	: GBase(gopts, GSPLASH_LOGGER) {
	QPixmap pixmap;

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


	if (!pixmap.isNull()) {
		splash = std::make_unique<QSplashScreen>(pixmap);
		splash->show();
		QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
	}
	// else splash remains null; caller can check isActive() if you expose one
}


void GSplash::messageAfter(int delay_ms, const std::string& msg) {
	if (!splash) return;

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
