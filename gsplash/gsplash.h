#pragma once

// gemc
#include "glogger.h"

// Qt
#include <QtWidgets>

#define GSPLASHENVIRONMENT    "GSPLASH"
#define NOSPLASHIMAGESELECTED "NOSPLASHIMAGESELECTED"

// exit codes: 190s
#define ERR_NOSPLASHENVFOUND  191

constexpr const char* GSPLASH_LOGGER = "gsplash";

namespace gsplash {
inline GOptions defineOptions() { return GOptions(GSPLASH_LOGGER); }
}

class GSplash {
private:
	QSplashScreen*           splash;
	std::shared_ptr<GLogger> log;

public:
	explicit GSplash(std::shared_ptr<GLogger> logger, const std::string& imageName = NOSPLASHIMAGESELECTED);

	~GSplash();

	void message(const std::string& message);

	// called in program using GSplash
	// returns focus to program window
	void finish(QWidget* callingWindow) { if (splash != nullptr) { splash->finish(callingWindow); } }

};
