#ifndef  GFIELDOPTIONS_H
#define  GFIELDOPTIONS_H 1

// glibrary
#include "goptions.h"
#include "gfield_multipoles.h"

namespace gfield {

    vector<GField_MultipolesFactory> get_GField_MultipolesFactories(GOptions *gopts);

    GOptions defineOptions();

}


#endif
