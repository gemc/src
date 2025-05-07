#pragma once

// glibrary
#include "goptions.h"
#include "gfield_multipoles.h"

namespace gfields {

    vector<GFieldDefinition> get_GFieldDefinition(GOptions *gopts);

    GOptions defineOptions();
}



