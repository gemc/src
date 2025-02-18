// gparticle
#include "gparticleOptions.h"

// c++
#include <iostream>
using namespace std;

int main(int argc, char *argv[]) {

    GOptions *gopts = new GOptions(argc, argv, gparticle::defineOptions());
	cout << " > Nthreads: " << gopts->getScalarInt("nthreads") << endl;

    return EXIT_SUCCESS;
}
