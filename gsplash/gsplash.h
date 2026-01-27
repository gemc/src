#pragma once

// gemc
#include "gbase.h"

// Qt
#include <QtWidgets>
#include <QTimer>

#define GSPLASHENVIRONMENT    "GSPLASH"
#define NOSPLASHIMAGESELECTED "NOSPLASHIMAGESELECTED"

// exit codes: 190s
#define ERR_NOSPLASHENVFOUND  191

constexpr const char* GSPLASH_LOGGER = "gsplash";

namespace gsplash {
inline GOptions defineOptions() { return GOptions(GSPLASH_LOGGER); }
}

class GSplash : public GBase<GSplash> {
public:
	// returns nullptr when GUI is off
	static std::unique_ptr<GSplash>
	create(const std::shared_ptr<GOptions>& gopts,
	       const std::string&               imageName = "gemcArchitecture");

	void message(const std::string& msg);
	void messageAfter(int delay, const std::string& msg);

	// called in program using GSplash
	// returns focus to the program window
	void finish(QWidget* callingWindow) const { if (splash != nullptr) { splash->finish(callingWindow); } }

private:
	// keep the constructor private, so callers must use create()
	GSplash(const std::shared_ptr<GOptions>& gopts,
	        const std::string&               imageName);

	std::unique_ptr<QSplashScreen> splash;

};
