#ifndef DOXYGEN_SHOULD_SKIP_THIS

#include "gworld.h"
#include "goptions.h"

// the geometry comes linked from one of the examples
int main(int argc, char *argv[]) {

	auto gopts = new GOptions(argc, argv, gsystem::defineOptions());

	auto world = new GWorld(gopts);

	delete world;
	return EXIT_SUCCESS;
}

#endif
