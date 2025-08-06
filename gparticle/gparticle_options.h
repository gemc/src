#pragma once

// gparticle
#include "gparticle.h"

// gemc
#include "goptions.h"

constexpr const char* GPARTICLE_LOGGER = "gparticle";

namespace gparticle {

// returns array of options definitions
GOptions defineOptions();

// method to return a vector of JSystem from a structured option
std::vector<GparticlePtr> getGParticles(const std::shared_ptr<GOptions>& gopts, std::shared_ptr<GLogger>& logger);

}
