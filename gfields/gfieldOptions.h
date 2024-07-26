#ifndef  GFIELDOPTIONS_H
#define  GFIELDOPTIONS_H 1

// glibrary
#include "goptions.h"
#include "gfield_multipoles.h"

namespace gfield {

    vector<GFieldDefinition> get_GFieldDefinition(GOptions *gopts);

    GOptions defineOptions();
}


#endif
