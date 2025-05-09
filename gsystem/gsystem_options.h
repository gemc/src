#pragma once

// gemc
#include "goptions.h"
#include "gsystem.h"
#include "gmodifier.h"
#include "glogger.h"

constexpr const char *GSYSTEM_LOGGER = "gsystem";

// using unique pointers to move GSystem to avoid unnecessary copies
using SystemPtr  = std::unique_ptr<GSystem>;
using SystemList = std::vector<SystemPtr>;

namespace gsystem {

// method to return a vector of GSystem from the options
SystemList getSystems(GOptions *gopts, const std::shared_ptr<GLogger>& log);

// method to return a vector of GModifier from the options
vector<GModifier> getModifiers(GOptions *gopts);

// returns array of options definitions
  GOptions defineOptions();

}

