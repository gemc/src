#pragma once

// gemc
#include "goptions.h"
#include "gsystem.h"
#include "gmodifier.h"
#include "glogger.h"

constexpr const char* GSYSTEM_LOGGER = "gsystem";

namespace gsystem {

// method to return a vector of GSystem from the options
SystemList getSystems(const std::shared_ptr<GOptions>& gopts, const std::shared_ptr<GLogger>& log);

// method to return a vector of GModifier from the options
std::vector<GModifier> getModifiers(const std::shared_ptr<GOptions>& gopts);

// returns array of options definitions
GOptions defineOptions();

}
