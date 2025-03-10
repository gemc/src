#ifndef  GREADOUTSPECS_H
#define  GREADOUTSPECS_H 1

// glibrary
#include "ghitConventions.h"

#include <math.h>  /* floor */

// properties of the readout electronics, used in GTouchable and GHit.
// they are defined here so that the plugin can define it
class GReadoutSpecs {

private:
	// readout specs, set by plugin function loadReadoutSpecs
	float     timeWindow;       // electronic readout time-window of the detector
	float     gridStartTime;    // defines the windows grid
	HitBitSet hitBitSet;        // defines what information to be stored in the hit
	bool      verbosity;

public:

	// constructor set by plugins
	GReadoutSpecs(const float tw, const float gst, const HitBitSet hbs, bool v = false) :
	timeWindow(tw),
	gridStartTime(gst),
	hitBitSet(hbs),
	verbosity(v) {
		// if(verbosity) {
		// }
	}

public:
	
	inline const HitBitSet getHitBitSet() const { return hitBitSet; }

	// for readout detectors, return the index time cell in the timewindow
	inline int timeCellIndex(float time) const {
		return (int) (floor((time - gridStartTime)/timeWindow) + 1);
	}
	
//	vector<string> showParameters() {
//		vector<string> messages;
//		messages.push_back(" Time Window: " + to_string(timeWindow));
//		return messages;
//	}
};


#endif
