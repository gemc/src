#include "gfactory_options.h"

// project goption to a system
namespace gfactory {

// returns array of options definitions
GOptions defineOptions() {

	GOptions goptions("plugins");
	goptions.defineSwitch("showPredefinedMaterials", "Log GEMC Predefined Materials");
	goptions.defineSwitch("printSystemsMaterials", "Print the materials used in this simulation");
	return goptions;
}


}
