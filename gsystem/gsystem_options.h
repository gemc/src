#pragma once

// gemc
#include "goptions.h"
#include "gsystem.h"
#include "gmodifier.h"
#include "glogger.h"

constexpr const char *GSYSTEM_LOGGER = "gsystem";

namespace gsystem {

// method to return a vector of GSystem from the options
vector<GSystem> getSystems(GOptions *gopts, const std::shared_ptr<GLogger>& log);

// method to return a vector of GModifier from the options
vector<GModifier> getModifiers(GOptions *gopts);

// returns array of options definitions
  GOptions defineOptions();

}

