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
		pixmap.load((":" + imageName).c_str());
		if (pixmap.isNull())
			log->error(ERR_NOSPLASHENVFOUND, "Image ", imageName, " not found.");
	}

	if (!pixmap.isNull()) {
		splash = std::make_unique<QSplashScreen>(pixmap);
		splash->show();
	}
	// else splash remains null; caller can check isActive() if you expose one
}

void GSplash::message(const string& msg) { if (splash) splash->showMessage(msg.c_str(), Qt::AlignLeft, Qt::black); }
