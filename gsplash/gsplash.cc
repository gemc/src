// splash
#include "gsplash.h"

GSplash::GSplash(std::shared_ptr<GLogger> logger, const string& imageName) : splash(nullptr), log(logger) {
	log->debug(CONSTRUCTOR, "GSplash");

	// no argument, imageName is defaulted at NOSPLASHIMAGESELECTED
	if (imageName == NOSPLASHIMAGESELECTED) {
		auto filename = getenv(GSPLASHENVIRONMENT); // char*

		if (filename != nullptr) {
			// pixmap is empty if the filename doesn't exist.
			QPixmap pixmap(filename);
			splash = new QSplashScreen(pixmap);
		}
		else { log->error(ERR_NOSPLASHENVFOUND, "environment variable ", GSPLASHENVIRONMENT, " must point to an image file."); }
	}
	else {
		string resourceImage = ":" + imageName;

		QPixmap pixmap(resourceImage.c_str());

		// check that the image exists
		if (pixmap.isNull()) { log->error(ERR_NOSPLASHENVFOUND, "Image ", imageName, " not found."); }
		splash = new QSplashScreen(pixmap);
	}

	splash->show();
}

void GSplash::message(const string& msg) { if (splash != nullptr) { splash->showMessage(msg.c_str(), Qt::AlignLeft, Qt::black); } }

GSplash::~GSplash() {
	log->debug(DESTRUCTOR, "GSplash");
	delete splash;
}
