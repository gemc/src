#ifndef  G4DISPLAY_H_OPTIONS
#define  G4DISPLAY_H_OPTIONS 1


// glibrary
#include "goptions.h" 

// g4display
#include "g4Text.h"


namespace g4display {

// G4View
struct JView {
	string viewer;
	string dimension;
	string position;
	int segsPerCircle;
};

void from_json(const json& j, JView& jview);
JView getJView(GOptions *gopts);


// G4Camera
struct JCamera {
	string phi;
	string theta;
};

void from_json(const json& j, JCamera& jcamera);
JCamera getJCamera(GOptions *gopts);


// Dawn
struct JDawn {
	string phi;
	string theta;
};

void from_json(const json& j, JDawn& jdawn);
JDawn getJDawn(GOptions *gopts);



// returns the array of options definitions
vector<GOption> defineOptions();


}

#endif
