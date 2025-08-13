#pragma once

// gemc
#include "goptions.h"
#include "gsystem.h"
#include "gmodifier.h"
#include "glogger.h"

constexpr const char* GVOLUME_LOGGER = "gvolume";
constexpr const char* GMATERIAL_LOGGER = "gmaterial";
constexpr const char* GSYSTEM_LOGGER = "gsystem";
constexpr const char* GWORLD_LOGGER = "gworld";

namespace gsystem {

// method to return a vector of GSystem from the options
SystemList getSystems(const std::shared_ptr<GOptions>& gopts);

// method to return a vector of GModifier from the options
std::vector<GModifier> getModifiers(const std::shared_ptr<GOptions>& gopts);

// returns array of options definitions
GOptions defineOptions();

}
