#include "gplugin_test_example.h"


bool GPlugin_test_example::defineReadoutSpecsImpl() {
	check_if_log_defined();

	double timeWindow    = 10;                  // electronic readout time-window of the detector
	double gridStartTime = 0;                   // defines the windows grid
	auto   hitBitSet     = HitBitSet("100000"); // defines what information to be stored in the hit

	readoutSpecs = std::make_shared<GReadoutSpecs>(timeWindow, gridStartTime, hitBitSet, digi_logger);

	return true;
}


bool GPlugin_test_example::loadConstantsImpl(int runno, [[maybe_unused]] std::string const& variation) {

	var1    = 2.0;
	var2[0] = 1;
	var2[0] = 2;

	var3.push_back(3.0);
	var3.push_back(4.0);
	var3.push_back(5.0);
	var3.push_back(6.0);

	var4 = "hello";

	digi_logger->info(0, " Constants loaded for run number ", runno, " for ctof. var1  is ", var1,
		", var2 pointer is ", var2, ", variation is ", variation);

	return true;
}

GDigitizedData* GPlugin_test_example::digitizeHitImpl([[maybe_unused]] GHit* ghit, [[maybe_unused]] size_t hitn) {

	// return a new GDigitizedData object with some dummy data derived from the hit
	auto digitizedData = std::make_unique<GDigitizedData>(ghit, digi_logger);

	auto edep = ghit->getTotalEnergyDeposited();

	double digi_time = 0;

	for (auto& time: ghit->getTimes()) {
		digi_time += time*10;
	}

	digitizedData->includeVariable("edep", edep);
	digitizedData->includeVariable("digi_time", digi_time);

	return digitizedData;
}


// tells the DLL how to create a GDynamicFactory
extern "C" GDynamicDigitization* GDynamicFactory(void) { return static_cast<GDynamicDigitization*>(new GPlugin_test_example); }
