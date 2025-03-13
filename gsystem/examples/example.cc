#ifndef DOXYGEN_SHOULD_SKIP_THIS

#include "gutilities.h"
#include "gworld.h"
#include "goptions.h"

using namespace std;

// the geometry comes from clas12-systems
// sqlite test temp disable due to clas12-system api needs to be revisited
//- name: cloudc
//		factory: sqlite
//		runno: 22

int main(int argc, char *argv[]) {
	GOptions *gopts = new GOptions(argc, argv, gsystem::defineOptions());

	GWorld *world = new GWorld(gopts);
	delete world;

	return EXIT_SUCCESS;
}


#endif
