// gemc
#include "gsd.h"

// this is thread-local
bool GSensitiveDetector::decisionToSkipHit(double energy)
{

	// PRAGMA TODO: call digi skipProcess?
	// in that case, need more argument, like trk

	if ( energy == 0 && !recordZeroEdep ) {
		return true;
	}

	return false;
}
