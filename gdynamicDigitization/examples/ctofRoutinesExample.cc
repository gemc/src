#include "ctofRoutinesExample.h"

#include <iostream>

using namespace std;


bool CTofRoutineExample::defineReadoutSpecs() {

    float timeWindow = 10;                  // electronic readout time-window of the detector
    float gridStartTime = 0;                // defines the windows grid
    HitBitSet hitBitSet = HitBitSet("100000");  // defines what information to be stored in the hit
    bool verbosity = true;

    readoutSpecs = new GReadoutSpecs(timeWindow, gridStartTime, hitBitSet, verbosity);

    return true;
}


bool CTofRoutineExample::loadConstants(int runno, [[maybe_unused]] string variation) {
    var1 = 2.0;
    var2[0] = 1;
    var2[0] = 2;

    var3.push_back(3.0);
    var3.push_back(4.0);
    var3.push_back(5.0);
    var3.push_back(6.0);

    var4 = "hello";

    cout << " Constants loaded for run number " << runno << " for ctof! var1  is " << var1 << " var2 pointer is " << var2 << endl;

    return true;
}

// tells the DLL how to create a GDynamicFactory
extern "C" GDynamicDigitization *GDynamicFactory(void) {
    return static_cast<GDynamicDigitization *>(new CTofRoutineExample);
}
