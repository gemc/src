// gparticle
#include "gparticle_options.h"

// c++
#include <iostream>
using namespace std;

int main(int argc, char *argv[]) {

    auto gopts = new GOptions(argc, argv, gparticle::defineOptions());


    return EXIT_SUCCESS;
}
