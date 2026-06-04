// Option composition for the GTree module.
// Doxygen documentation is in gtree_options.h (header is authoritative).

// gemc
#include "dbselect_options.h"
#include "gtree_options.h"

namespace gtree {

// Build the option set for the gtree module by aggregating dbselect options.
GOptions defineOptions() {
	GOptions goptions(GTREE_LOGGER);
	goptions += dbselect::defineOptions();

	return goptions;
}
}
