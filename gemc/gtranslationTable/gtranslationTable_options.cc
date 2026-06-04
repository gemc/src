#include "gtranslationTable_options.h"

// project goption to a system
namespace gtranslationTable {
// returns array of options definitions
GOptions defineOptions() {
	// Create an option container bound to the module logger name.
	// Add module-specific options here as the module grows.
	GOptions goptions(TRANSLATIONTABLE_LOGGER);

	return goptions;
}
}
