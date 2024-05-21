#ifndef  G4DISPLAY_H_OPTIONS
#define  G4DISPLAY_H_OPTIONS 1

// gemc
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
    JView getJView(GOptions *gopts);


    // G4Camera
    struct JCamera {
        string phi;
        string theta;
    };
    JCamera getJCamera(GOptions *gopts);


    // Dawn
    struct JDawn {
        string phi;
        string theta;
    };
    JDawn getJDawn(GOptions *gopts);


    vector <GOption> defineOptions();
}

#endif
