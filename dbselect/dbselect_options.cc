#include "dbselect_options.h"

// gemc
#include "gdetector_options.h"

// namespace to define options
namespace dbselect {

    GOptions defineOptions() {

        GOptions goptions(DBSELECT_LOGGER);

		goptions += gdetector::defineOptions();

        return goptions;
    }

}
