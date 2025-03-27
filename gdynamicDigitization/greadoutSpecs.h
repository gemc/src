#ifndef  GREADOUTSPECS_H
#define  GREADOUTSPECS_H 1

// gemc for HitBitSet
#include "ghitConventions.h"
#include "glogger.h"

#include <math.h>  /* floor */

// properties of the readout electronics, used in GTouchable and GHit.
// they are defined here so that the plugin can define it
class GReadoutSpecs {

private:
	// readout specs, set by plugin function loadReadoutSpecs
	float timeWindow;       // electronic readout time-window of the detector
	float gridStartTime;    // defines the windows grid
	HitBitSet hitBitSet;        // defines what information to be stored in the hit

public:

	// constructor set by plugins
	GReadoutSpecs(const float tw, const float gst, const HitBitSet hbs, GLogger *log) :
			timeWindow(tw),
			gridStartTime(gst),
			hitBitSet(hbs) {
		log->info(1, "GReadoutSpecs: timeWindow=", timeWindow, ", gridStartTime=", gridStartTime, ", hitBitSet=", hitBitSet);
	}

public:

	inline const HitBitSet getHitBitSet() const { return hitBitSet; }

	// for readout detectors, return the index time cell in the timewindow
	inline int timeCellIndex(float time) const {
		return (int) (floor((time - gridStartTime) / timeWindow) + 1);
	}
	
};


#endif
