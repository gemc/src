#include "ginternalDigitization.h"


bool GParticleCounterDigitization::defineReadoutSpecsImpl() {
	float timeWindow    = 10;                  // electronic readout time-window of the detector
	float gridStartTime = 0;                   // defines the window grid
	auto  hitBitSet     = HitBitSet("000000"); // defines what information to be stored in the hit

	readoutSpecs = new GReadoutSpecs(timeWindow, gridStartTime, hitBitSet, digi_logger);

	return true;
}


// digitized the hit
GDigitizedData* GParticleCounterDigitization::digitizeHitImpl(GHit* ghit, size_t hitn) {
	// ghit->getGID() must have a single entry
	GIdentifier identity = ghit->getGID().front();

	auto gdata = new GDigitizedData(ghit, data_logger);

	gdata->includeVariable(identity.getName(), identity.getValue());
	gdata->includeVariable("hitn", static_cast<int> (hitn) );
	gdata->includeVariable("totEdep", ghit->getTotalEnergyDeposited());
	gdata->includeVariable("time", ghit->getAverageTime());
	gdata->includeVariable("pid", ghit->getPid());
	gdata->includeVariable("totalE", ghit->getE());

	return gdata;
}
