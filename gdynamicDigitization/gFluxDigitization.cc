#include "ginternalDigitization.h"

// See header for API docs.
bool GFluxDigitization::defineReadoutSpecsImpl() {
	check_if_log_defined();

	double    timeWindow    = 10;  // electronic readout time-window of the detector
	double    gridStartTime = 0;   // defines the windows grid
	HitBitSet hitBitSet("000001"); // defines what information to be stored in the hit

	readoutSpecs = std::make_shared<GReadoutSpecs>(timeWindow, gridStartTime, hitBitSet, log);

	return true;
}

// See header for API docs.
std::unique_ptr<GDigitizedData> GFluxDigitization::digitizeHitImpl(GHit* ghit, size_t hitn) {
	check_if_log_defined();

	GIdentifier identity = ghit->getGID().front();

	auto gdata = std::make_unique<GDigitizedData>(gopts, ghit);

	gdata->includeVariable(identity.getName(), identity.getValue());
	gdata->includeVariable("hitn", static_cast<int>(hitn));
	gdata->includeVariable("totEdep", ghit->getTotalEnergyDeposited());
	gdata->includeVariable("time", ghit->getAverageTime());
	gdata->includeVariable("pid", ghit->getPid());
	gdata->includeVariable("totalE", ghit->getE());

	return gdata;
}
