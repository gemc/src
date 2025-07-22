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
	bool loadTTImpl([[maybe_unused]] int runno, [[maybe_unused]] std::string const& variation) override;

	[[nodiscard]]std::unique_ptr<GDigitizedData> digitizeHitImpl(const std::unique_ptr<GHit>& ghit, [[maybe_unused]] size_t hitn) override;

private:

	double var1 = 1;
	int var2[2] = {0, 0}; // array of integers
	std::vector<double> var3;
	std::string var4;

	std::shared_ptr<GTranslationTable> translationTable;
};

extern "C" GDynamicDigitization* GDynamicDigitizationFactory(void) {
    return static_cast<GDynamicDigitization*>(new GPlugin_test_example);
}

