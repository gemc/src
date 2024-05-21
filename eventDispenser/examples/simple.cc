// eventDispenser
#include "eventDispenser.h"
#include "eventDispenserOptions.h"

// goptions
#include "goptions.h"


// c++
#include <iostream>

using namespace std;


int main(int argc, char *argv[]) {

    GOptions *gopts = new GOptions(argc, argv, eventDispenser::defineOptions());

    cout << " Event Dispenser Verbosity: " << gopts->getVerbosityFor("event_dispenser") << endl;


    return EXIT_SUCCESS;
}
