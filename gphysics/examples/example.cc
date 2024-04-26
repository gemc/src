// gphysics
#include "gphysics.h"
#include "gphysicsOptions.h"

// goptions
#include "goptions.h"

// c++
#include <iostream>
using namespace std;


int main(int argc, char* argv[])
{
	GOptions *gopts = new GOptions(argc, argv, gphysics::defineOptions());
	gopts->printSettings(false);
	
	[[maybe_unused]] auto gphysics = new GPhysics(gopts);

	return EXIT_SUCCESS;
}
