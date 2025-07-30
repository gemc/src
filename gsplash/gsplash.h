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

class GSplash
{
public:
	// returns nullptr when GUI is off
	static std::unique_ptr<GSplash>
	create(const std::shared_ptr<GOptions>& gopts,
		   const std::string&               imageName = "gemcArchitecture");

	void message(const std::string& msg);
	~GSplash() = default;

	// called in program using GSplash
	// returns focus to program window
	void finish(QWidget* callingWindow) { if (splash != nullptr) { splash->finish(callingWindow); } }

private:
	// keep constructor private so callers must use create()
	GSplash(const std::shared_ptr<GOptions>& gopts,
			const std::string&               imageName);

	std::unique_ptr<QSplashScreen> splash;
	std::shared_ptr<GLogger>       log;

};