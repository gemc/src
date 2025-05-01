#ifndef  G4DISPLAY_H_OPTIONS
#define  G4DISPLAY_H_OPTIONS 1

// gemc
#include "goptions.h"


constexpr const char* G4DISPLAY_LOGGER = "g4display";
constexpr const char* G4SCENE_LOGGER   = "g4scene";

namespace g4display {

// G4View
struct G4View {
	string driver;
	string dimension;
	string position;
	int    segsPerCircle;
};

G4View getG4View(GOptions* gopts);


// G4Camera
struct G4Camera {
	string phi;
	string theta;
};

G4Camera getG4Camera(GOptions* gopts);

// Dawn
struct G4Dawn {
	string phi;
	string theta;
};

G4Dawn getG4Dawn(GOptions* gopts);

GOptions defineOptions();
}

#endif
