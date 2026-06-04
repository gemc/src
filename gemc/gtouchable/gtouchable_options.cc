#include "gtouchable_options.h"

// project goption to a system
namespace gtouchable {
// See header for API docs.

// returns array of options definitions
GOptions defineOptions() {
	GOptions goptions(TOUCHABLE_LOGGER);

	return goptions;
}
}
