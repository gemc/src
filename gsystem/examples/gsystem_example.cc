#include "gworld.h"

// the geometry comes linked from one of the examples
int main(int argc, char *argv[]) {

	auto gopts =std::make_shared<GOptions>(argc, argv, gsystem::defineOptions());
	auto world = new GWorld(gopts);

	delete world;
	return EXIT_SUCCESS;
}

