#include "ginternalDigitization.h"

bool GFluxDigitization::defineReadoutSpecs() {
	float     timeWindow = 10;                  // electronic readout time-window of the detector
	float     gridStartTime = 0;                // defines the windows grid
	HitBitSet hitBitSet = HitBitSet("000001");  // defines what information to be stored in the hit
	bool      verbosity = true;

	readoutSpecs = new GReadoutSpecs(timeWindow, gridStartTime, hitBitSet, verbosity);

	return true;
}


// digitized the hit
GDigitizedData* GFluxDigitization::digitizeHit(GHit *ghit, size_t hitn) {

	// ghit->getGID() must have a single entry
	GIdentifier identity = ghit->getGID().front();

	GDigitizedData* gdata = new GDigitizedData(ghit);

	gdata->includeVariable(identity.getName(), identity.getValue()             );
	gdata->includeVariable("hitn",             (int) hitn                      );
	gdata->includeVariable("totEdep",          ghit->getTotalEnergyDeposited() );
	gdata->includeVariable("time",             ghit->getAverageTime()          );
	gdata->includeVariable("pid",              ghit->getPid()                  );
	gdata->includeVariable("totalE",           ghit->getE()                    );

	return gdata;
}


