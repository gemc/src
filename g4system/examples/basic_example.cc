// g4system
#include "g4systemOptions.h"

// gemc
#include "gutilities.h"
using namespace std;

int main(int argc, char* argv[]) {

    GOptions *gopts = new GOptions(argc, argv, g4system::defineOptions());

	cout << " > Nthreads: " << gopts->getScalarInt("nthreads") << endl;

    return EXIT_SUCCESS;
}
