#ifndef  GSTREAMEROPTIONS_H
#define  GSTREAMEROPTIONS_H 1

// gstreamer
#include "gstreamer.h"

// glibrary
#include "goptions.h"

namespace gstreamer {

    // method to return a vector of GOutputDefinition from the options
    vector<GOutputDefinition> getGOutputDefinition(GOptions *gopts);

    // returns array of options definitions
    GOptions defineOptions();
}


#endif
