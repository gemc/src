// c++
#include <iostream>
using namespace std;

// splash
#include "gsplash.h"

GSplash::GSplash(string imageName) : splash(nullptr)
{
	// no argument, imageName is defaulted at NOSPLASHIMAGESELECTED
	if ( imageName == NOSPLASHIMAGESELECTED ) {

		auto filename = getenv(GSPLASHENVIRONMENT); // char*

		if ( filename != nullptr) {
			// pixmap is empty if filename doesn't exist.
			QPixmap pixmap(filename);
			splash = new QSplashScreen(pixmap);
		} else {
			cerr << FATALERRORL << "environment variable " << GSPLASHENVIRONMENT << " must point to an image file." << endl;
			gexit(EC__NOSPLASHENVFOUND);
		}

		cout << " TODO: Need to add gutilities and goptions but loading image from environment" << endl;

	} else {
		string resourceImage = ":" + imageName;

		QPixmap pixmap(resourceImage.c_str());

		// check that the image exists
		if (pixmap.isNull()) {
			cerr << FATALERRORL << "Image " << imageName << " not found." << endl;
			gexit(EC__NOSPLASHENVFOUND);
		}
		splash = new QSplashScreen(pixmap);
    }

	if (splash != nullptr) {
		splash->show();
	}
}

void GSplash::message(string msg) {
	if(splash != nullptr) {
		splash->showMessage(msg.c_str(),  Qt::AlignLeft,  Qt::black );
	}
}
