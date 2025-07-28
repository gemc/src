#include "gdetector_options.h"
#include "gsystem_options.h"
#include "g4system_options.h"
#include "gdynamicdigitization_options.h"


// namespace to define options
namespace gdetector {

    GOptions defineOptions() {

        GOptions goptions(GDETECTOR_LOGGER);

		goptions += gsystem::defineOptions();
    	goptions += g4system::defineOptions();
    	goptions += gdynamicdigitization::defineOptions();

        return goptions;
    }

}
