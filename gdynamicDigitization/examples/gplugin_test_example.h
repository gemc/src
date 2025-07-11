#pragma once

// gdynamic
#include "gdynamicdigitization.h"

// c++
#include <string>

class GPlugin_test_example : public GDynamicDigitization {

public:
    // constructor is not needed

    // mandatory readout specs definitions
	bool defineReadoutSpecsImpl() override;

	bool loadConstantsImpl(int runno, std::string const &variation) override;
	[[nodiscard]] GDigitizedData* digitizeHitImpl([[maybe_unused]] GHit* ghit, [[maybe_unused]] size_t hitn) override;

private:

	double var1 = 1;
	int var2[2] = {0, 0}; // array of integers
	std::vector<double> var3;
	std::string var4;
};

extern "C" GDynamicDigitization* GDynamicDigitizationFactory(void) {
    return static_cast<GDynamicDigitization*>(new GPlugin_test_example);
}

