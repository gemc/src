#ifndef GPLUGIN_TEST_EXAMPLE
#define GPLUGIN_TEST_EXAMPLE 1

// gdynamic
#include "gdynamicdigitization.h"

// c++
#include <string>

class GPlugin_test_example : public GDynamicDigitization {

public:
    // constructor not needed

    // mandatory readout specs definitions
	bool defineReadoutSpecs();

	bool loadConstants(int runno, string variation);

private:

	double var1;
	int var2[2];
	vector<float> var3;
	string var4;
};

extern "C" GDynamicDigitization* GDynamicDigitizationFactory(void) {
    return static_cast<GDynamicDigitization*>(new GPlugin_test_example);
}

#endif
