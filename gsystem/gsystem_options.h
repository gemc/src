#ifndef  GSYSTEMOPTIONS_H
#define  GSYSTEMOPTIONS_H 1

// glibrary
#include "goptions.h"
#include "gsystem.h"
#include "gmodifier.h"

constexpr const char *GSYSTEM_LOGGER = "gsystem";

namespace gsystem {

// method to return a vector of GSystem from the options
vector<GSystem> getSystems(GOptions *gopts);

// method to return a vector of GModifier from the options
vector<GModifier> getModifiers(GOptions *gopts);

// returns array of options definitions
GOptions defineOptions();

}


#endif
