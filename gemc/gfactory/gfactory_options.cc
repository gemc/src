#include "gfactory_options.h"

// project goption to a system
namespace gfactory {

GOptions defineOptions() {

	GOptions goptions(PLUGIN_LOGGER);

	goptions.defineOption(
		GVariable("plugin_path", "", "colon-separated list of directories to search for .gplugin files"),
		"Additional directories searched for GEMC plugin libraries (*.gplugin) before the\n"
		"current working directory and the system library path (LD_LIBRARY_PATH / DYLD_LIBRARY_PATH).\n"
		"Example: -plugin_path=/opt/clas12/lib:/usr/local/gemc/plugins"
	);

	return goptions;
}

}
