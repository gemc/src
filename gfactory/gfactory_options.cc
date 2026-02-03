#include "gfactory_options.h"

// project goption to a system
namespace gfactory {

// returns array of options definitions
GOptions defineOptions() {

	// Construct the options container for the plugin logger channel.
	// Individual flags can be added here as the module evolves.
	GOptions goptions(PLUGIN_LOGGER);

	return goptions;
}

}
