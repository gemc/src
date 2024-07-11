#ifndef  GPARTICLE_H_OPTIONS
#define  GPARTICLE_H_OPTIONS 1

// glibrary
#include "goptions.h"
#include "gparticle.h"

namespace gparticle {

    // returns array of options definitions
    GOptions defineOptions();

    // method to return a vector of JSystem from a structured option
    vector <Gparticle> getParticles(GOptions *gopts);

}


#endif
