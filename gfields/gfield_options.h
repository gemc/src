#pragma once

// gemc
#include "goptions.h"

// gfields
#include "gfield.h"


namespace gfields {

std::vector<GFieldDefinition> get_GFieldDefinition(const std::shared_ptr<GOptions>& gopts);

GOptions defineOptions();

}
