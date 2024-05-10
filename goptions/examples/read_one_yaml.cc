#ifndef DOXYGEN_SHOULD_SKIP_THIS

// goptions
#include "goptions.h"

// c++
#include <iostream>

using namespace std;

// returns this example options
GOptions defineOptions() {
    GOptions goptions;

    // command line switch
    goptions.addSwitch("batch", "run in batch mode");


    // runno: 12,
    // nthreads: 8
    goptions.addOption("runno", "run number", "run_number: 12", "run number");
    goptions.addOption("nthreads", "number of threads", "nthreads: 8", "number of threads");

     // map option
    // +gparticle:
    //    pname: "e-",
    //    multiplicity": 1
    //    p: 2300
    //    theta: 23.0
    vector<string> gparticle = {
        "pname: \"e-\"",
        "multiplicity: 1",
        "p: 2300",
        "theta: 23.0"
    };

    goptions.addOption("+gparticle", "generator particle", gparticle, "generator particle");

    return goptions;
}


int main(int argc, char *argv[]) {

    GOptions *gopts = new GOptions(argc, argv, defineOptions());

    gopts->print_help();

    return EXIT_SUCCESS;
}

#endif
