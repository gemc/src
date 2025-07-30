// gphysics
#include "gphysics.h"
#include "gphysics_options.h"

// goptions
#include "goptions.h"


int main(int argc, char* argv[]) {
	auto gopts =  std::make_shared<GOptions>(argc, argv, gphysics::defineOptions());

	[[maybe_unused]] auto gphysics = std::make_shared<GPhysics>(gopts);

	return EXIT_SUCCESS;
}
