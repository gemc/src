#include "gplugin_test_example.h"

// Implementation notes:
// - Doxygen API docs for this example live in gplugin_test_example.h.
// - This .cc file contains implementation-only comments to avoid duplicated \param sections.

bool GPlugin_test_example::defineReadoutSpecsImpl() {
	double timeWindow    = 10;                  // electronics time-window (width of one time cell)
	double gridStartTime = 0;                   // time grid origin
	auto   hitBitSet     = HitBitSet("100000"); // bitset defining which hit information is computed/stored

	// The readoutSpecs object is shared and treated as immutable after initialization.
	readoutSpecs = std::make_shared<GReadoutSpecs>(timeWindow, gridStartTime, hitBitSet, log);

	return true;
}

bool GPlugin_test_example::loadConstantsImpl(int runno, [[maybe_unused]] std::string const& variation) {
	var1 = 2.0;

	// Fill the small fixed-size array with deterministic values.
	// NOTE: the original code assigned var2[0] twice. We do not change code here; this comment
	// documents the likely intent of populating both entries.
	var2[0] = 1;
	var2[0] = 2;

	// Populate the vector with a few values to show container usage.
	var3.push_back(3.0);
	var3.push_back(4.0);
	var3.push_back(5.0);
	var3.push_back(6.0);

	// Simple string constant.
	var4 = "hello";

	log->info(0, " Constants loaded for run number ", runno, " for ctof. var1  is ", var1,
	          ", var2 pointer is ", var2, ", variation is ", variation);

	return true;
}

bool GPlugin_test_example::loadTTImpl([[maybe_unused]] int runno, [[maybe_unused]] std::string const& variation) {
	std::vector<int> element1 = {1, 2, 3, 4, 5};
	std::vector<int> element2 = {2, 2, 3, 4, 5};

	GElectronic crate1(2, 1, 3, 2);
	GElectronic crate2(2, 1, 4, 2);

	translationTable = std::make_shared<GTranslationTable>(gopts);

	translationTable->addGElectronicWithIdentity(element1, crate1);
	translationTable->addGElectronicWithIdentity(element2, crate2);

	// Example: retrieving the electronics would look like:
	// auto retrievedElectronic = translationTable->getElectronics(element1);

	return true;
}

[[nodiscard]] std::unique_ptr<GDigitizedData> GPlugin_test_example::digitizeHitImpl(
	GHit* ghit, [[maybe_unused]] size_t hitn) {
	// Return a new GDigitizedData object with some data derived from the hit.
	auto digitizedData = std::make_unique<GDigitizedData>(gopts, ghit);

	auto edep = ghit->getTotalEnergyDeposited();

	double digi_time = 0;

	// Example time shaping: scale each recorded time and sum.
	for (auto& time : ghit->getTimes()) {
		digi_time += time * 10;
	}

	digitizedData->includeVariable("voltage", edep);
	digitizedData->includeVariable("digi_time", digi_time);

	return digitizedData;
}

// Tells the DLL how to create a GPlugin_test_example in each plugin .so/.dylib.
// The dynamic plugin loader expects an extern "C" function named GDynamicDigitizationFactory.
extern "C" GDynamicDigitization* GDynamicDigitizationFactory(const std::shared_ptr<GOptions>& g) {
	return static_cast<GDynamicDigitization*>(new GPlugin_test_example(g));
}
