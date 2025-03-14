#include "dbselectOptions.h"
#include "gsystemOptions.h"


// namespace to define options
namespace dbselect {

    GOptions defineOptions() {

        GOptions goptions;

		goptions += gsystem::defineOptions();

        return goptions;
    }

}
