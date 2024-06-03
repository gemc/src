// g4system
#include "g4systemOptions.h"

// gemc
#include "gutilities.h"

int main(int argc, char* argv[]) {

    GOptions *gopts = new GOptions(argc, argv, g4system::defineOptions());

    return EXIT_SUCCESS;
}
