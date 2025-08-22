#pragma once

// gemc
#include "goptions.h"

constexpr const char* G4SYSTEM_LOGGER = "g4system";
constexpr const char* G4SFACTORY_LOGGER = "g4sfactory";

namespace g4system {

// returns array of options definitions
GOptions defineOptions();

}
