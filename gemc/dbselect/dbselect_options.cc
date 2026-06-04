#include "dbselect_options.h"

// gemc
#include "gdetector_options.h"

// dbselect option definition implementation (see header for authoritative documentation).
namespace dbselect {
GOptions defineOptions() {
	GOptions goptions(DBSELECT_LOGGER);

	goptions += gdetector::defineOptions();

	return goptions;
}
}
