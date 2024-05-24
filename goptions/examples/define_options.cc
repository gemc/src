#ifndef DOXYGEN_SHOULD_SKIP_THIS

// goptions
#include "goptions.h"

// c++
#include <iostream>

using namespace std;

// returns this example options
GOptions defineOptions() {
    GOptions goptions;
    string help;

    // command line switch
    goptions.defineSwitch("log", "a switch, this is just an example.");

    // runno: 12,
    help = "Example: -runno=12\n";
    goptions.defineOption(GVariable("runno", 1, "sets the run number"), help);

    // nthreads: 8
    help = "If not set, use all available threads. 0: use all threads\n";
    help = "Example: -nthreads=12\n";
    goptions.defineOption(GVariable("nthreads", 0, "number of threads"), help);

    vector <GVariable> gparticle = {
            {"pname",        goptions::NODFLT, "particle name"},
            {"multiplicity", 1,                "number of particles per event"},
            {"p",            goptions::NODFLT, "momentum"},
            {"theta",        "0*degrees",      "polar angle"}
    };

    help = "Example to add two particles: \n \n";
    help += "-gparticle=\"[{pname: e-, multiplicity: 1, p: 2300, theta: 23.0}, {pname: proton, multiplicity: 2, p: 1200, theta: 14.0}]\"\n";
    goptions.defineOption("gparticle", "define the generator particle(s)", gparticle, help);

    return goptions;
}

// Define options
int main(int argc, char *argv[]) {

    new GOptions(argc, argv, defineOptions());

    return EXIT_SUCCESS;
}

#endif
