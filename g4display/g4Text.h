#pragma once

// g4display
#include "g4displayConventions.h"

// gemc
#include "goptions.h"


namespace g4display {

// G4SceneText: Text does not move with the view but belongs to the scene
struct G4SceneText {
	std::string text;
	std::string color;
	double      x;
	double      y;
	double      z = GNOT_SPECIFIED_SCENE_TEXT_Z;
	int         size;
};

// method to return a vector of G4SceneText from the options
std::vector<G4SceneText> getSceneTexts(const std::shared_ptr<GOptions>& gopts);

// returns the array of options definitions
GOptions addSceneTextsOptions();

}
