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

    vector <GVariable> verbosity = {
            {"fields",    0, "verbosity for fields"},
            {"particles", 0, "verbosity for particles"},
            {"hits",      0, "verbosity for hits"},
            {"banks",     0, "verbosity for banks"},
            {"detectors", 0, "verbosity for detectors"},
            {"materials", 0, "verbosity for materials"},
            {"regions",   0, "verbosity for regions"},
            {"volumes",   0, "verbosity for volumes"},
            {"surfaces",  0, "verbosity for surfaces"},
            {"optical",   0, "verbosity for optical properties"},
            {"all",       0, "verbosity for all"}
    };

    help = "Effects: \n \n";
    help += "0: shush\n";
    help += "1: summaryt\n";
    help += "2: details\n";
    help += "3: everything\n";
    goptions.defineOption("verbosity", "Sets the log verbosity for various categories", verbosity, help);



    return goptions;
}

// Define options
int main(int argc, char *argv[]) {

    new GOptions(argc, argv, defineOptions());

    return EXIT_SUCCESS;
}

#endif
