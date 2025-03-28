#ifndef  G4SCENE_PROPERTIES
#define  G4SCENE_PROPERTIES  1


// c++
#include <string>
#include <vector>

// gemc
#include "glogger.h"

// class used to initialize G4UImanager
// notice: this can be 
class G4SceneProperties {

public:
    explicit G4SceneProperties(GOptions *gopts);

	// default destructor
    ~G4SceneProperties() = default;


    vector <string> addSceneTexts(GOptions *gopts);

private:
	GLogger * const log;
};


#endif
