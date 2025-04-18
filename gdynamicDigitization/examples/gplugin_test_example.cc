#include "gplugin_test_example.h"


bool GPlugin_test_example::defineReadoutSpecsImpl() {
	check_if_log_defined();

    float timeWindow = 10;                  // electronic readout time-window of the detector
    float gridStartTime = 0;                // defines the windows grid
    auto hitBitSet = HitBitSet("100000");  // defines what information to be stored in the hit

    readoutSpecs = new GReadoutSpecs(timeWindow, gridStartTime, hitBitSet, digi_logger);

    return true;
}


bool GPlugin_test_example::loadConstantsImpl(int runno, [[maybe_unused]] string const &variation) {
	check_if_log_defined();

    var1 = 2.0;
    var2[0] = 1;
    var2[0] = 2;

    var3.push_back(3.0);
    var3.push_back(4.0);
    var3.push_back(5.0);
    var3.push_back(6.0);

    var4 = "hello";

	digi_logger->info(0, " Constants loaded for run number ", runno, " for ctof. var1  is ", var1, " var2 pointer is ", var2, "variation is", variation);

    return true;
}

// tells the DLL how to create a GDynamicFactory
extern "C" GDynamicDigitization *GDynamicFactory(void) {
    return static_cast<GDynamicDigitization *>(new GPlugin_test_example);
}
