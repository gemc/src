// gemc
#include "dbselect_options.h"

#include "gtree_options.h"

namespace gtree {

    GOptions defineOptions() {
        GOptions goptions(GTREE_LOGGER);
        goptions += dbselect::defineOptions();

        return goptions;
    }


}
