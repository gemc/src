#ifndef DOXYGEN_SHOULD_SKIP_THIS /* DOXYGEN_SHOULD_SKIP_THIS */

// goptions
#include "goptions.h"

// c++
#include <iostream>

using namespace std;


// returns array of options definitions
vector<GOption> defineOptions() {
    vector<GOption> goptions;

    // add a "gui" switch
    goptions.push_back(GOption("gui", "Use Graphical User Interface"));

    // add a "runno" option
    // not groupable
    // no help, just description
    json jsonRunOption = {
            {GNAME, "runno"},
            {GDESC, "Sets the Run number"},
            {GDFLT, 11}
    };

    goptions.push_back(GOption(jsonRunOption));

    // add a "nthreads" option
    // not groupable
    // no help, just description
    json jsonNThreadsOption = {
            {GNAME, "nthreads"},
            {GDESC, "Sets the Number of threads"},
            {GDFLT, 4}
    };

    goptions.push_back(GOption(jsonNThreadsOption));

    return goptions;
}


int main(int argc, char *argv[]) {
    GOptions *gopts = new GOptions(argc, argv, defineOptions());

    // print settings wiwth defaults
    gopts->printSettings(true);


    // get option and print it from here
    int runno = gopts->getInt("runno");
    bool gui = gopts->getSwitch("gui");
    int nthreads = gopts->getInt("nthreads");

    cout << " simple runno: " << runno << ", gui: " << gui << ", nthreads: " << nthreads << endl;

    if (gui == false && nthreads == 8 && runno == 12) {
        return EXIT_SUCCESS;

    }
    return EXIT_FAILURE;

}

#endif /* DOXYGEN_SHOULD_SKIP_THIS */
