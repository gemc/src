#ifndef DEFINEOPTIONS_H
#define DEFINEOPTIONS_H 1

// goptions
#include "goptions.h"

namespace goptions {


}

// returns array of options definitions
// this calls all the static defineOptions functions in the various libraries/frameworks
vector<GOption> defineOptions();

#endif
