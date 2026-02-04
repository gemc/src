#include "ginternalDigitization.h"

// See header for API docs.
bool GParticleCounterDigitization::defineReadoutSpecsImpl() {
	double timeWindow    = 10;                  // electronic readout time-window of the detector
	double gridStartTime = 0;                   // defines the window grid
	auto   hitBitSet     = HitBitSet("000000"); // defines what information to be stored in the hit

	readoutSpecs = std::make_shared<GReadoutSpecs>(timeWindow, gridStartTime, hitBitSet, log);

	return true;
}

// See header for API docs.
std::unique_ptr<GDigitizedData> GParticleCounterDigitization::digitizeHitImpl(GHit* ghit, size_t hitn) {
	// Expected to be a single-identity detector: take the first identity entry.
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
