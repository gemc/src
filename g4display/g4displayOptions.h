#ifndef  G4DISPLAY_H_OPTIONS
#define  G4DISPLAY_H_OPTIONS 1

// gemc
#include "goptions.h"

// g4display
#include "g4Text.h"


namespace g4display {

    // G4View
    struct G4View {
        string viewer;
        string dimension;
        string position;
        int segsPerCircle;
    };

    G4View getG4View(GOptions *gopts);


    // G4Camera
    struct G4Camera {
        string phi;
        string theta;
    };

    G4Camera getG4Camera(GOptions *gopts);


    // Dawn
    struct G4Dawn {
        string phi;
        string theta;
    };

    G4Dawn getG4Dawn(GOptions *gopts);

    GOptions defineOptions();
}

#endif
