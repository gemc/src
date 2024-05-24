#ifndef  G4SCENE_PROPERTIES
#define  G4SCENE_PROPERTIES  1

// c++
#include <string>
#include <vector>

// glibrary
#include "goptions.h"

// class used to initialize G4UImanager
// notice: this can be 
class G4SceneProperties {

public:
    G4SceneProperties(GOptions *gopts);

    vector <string> addSceneTexts(GOptions *gopts);
};


#endif
