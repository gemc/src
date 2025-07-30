#pragma once

// gemc
#include "goptions.h"


constexpr const char* G4DISPLAY_LOGGER = "g4display";
constexpr const char* G4SCENE_LOGGER   = "g4scene";

namespace g4display {

// G4View
struct G4View {
	std::string driver;
	std::string dimension;
	std::string position;
	int         segsPerCircle;
};

G4View getG4View(const std::shared_ptr<GOptions>& gopts);


// G4Camera
struct G4Camera {
	std::string phi;
	std::string theta;
};

G4Camera getG4Camera(const std::shared_ptr<GOptions>& gopts);

// Dawn
struct G4Dawn {
	std::string phi;
	std::string theta;
};

G4Dawn getG4Dawn(const std::shared_ptr<GOptions>& gopts);

GOptions defineOptions();
}
