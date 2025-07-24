#pragma once

// gemc
#include "goptions.h"

// gfields
#include "gfield.h"


namespace gfields {

std::vector<GFieldDefinition> get_GFieldDefinition(GOptions * gopts);

GOptions defineOptions();

}
