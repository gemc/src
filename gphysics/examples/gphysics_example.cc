// gphysics
#include "gphysics.h"
#include "gphysics_options.h"

// goptions
#include "goptions.h"


int main(int argc, char* argv[]) {
	auto gopts = new GOptions(argc, argv, gphysics::defineOptions());
	auto log   = std::make_shared<GLogger>(gopts, GPHYSICS_LOGGER, "gphysics_example");

	[[maybe_unused]] auto gphysics = new GPhysics(gopts, log);

	delete gphysics;
	return EXIT_SUCCESS;
}
