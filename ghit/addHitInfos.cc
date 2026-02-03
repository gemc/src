// ghit
#include "ghit.h"

// geant4
#include "G4TouchableHistory.hh"
#include "G4VProcess.hh"

// See header for API docs.

void GHit::addHitInfosForBitset(const HitBitSet hbs, const G4Step* step) {
	// Preconditions:
	// - 'step' must be non-null (callers provide a valid G4Step from the stepping action).
	// - 'gtouchable' must be valid; it provides energy scaling and identity/dimensions.

	auto preStepPoint = step->GetPreStepPoint();
	// auto poststepPoint = step->GetPostStepPoint();

	auto touchable = preStepPoint->GetTouchable();

	// Get the global position and transform it to local coordinates of the touchable.
	G4ThreeVector xyz  = preStepPoint->GetPosition();
	G4ThreeVector xyzL = touchable->GetHistory()->GetTopTransform().TransformPoint(xyz);

	globalPositions.push_back(xyz);
	localPositions.push_back(xyzL);

	// Retrieve energy deposition and time information.
	// Energy is scaled by the detector-specific multiplier from GTouchable.
	double edep = (step->GetTotalEnergyDeposit()) * (gtouchable->getEnergyMultiplier());
	double time = preStepPoint->GetGlobalTime();

	edeps.push_back(edep);
	times.push_back(time);

	// Iterate over each bit and call the helper method to add optional info.
	for (size_t hbIndex = 0; hbIndex < hbs.size(); hbIndex++) {
		addHitInfosForBitIndex(hbIndex, hbs.test(hbIndex), step);
	}
}

bool GHit::addHitInfosForBitIndex(size_t bitIndex, const bool test, const G4Step* thisStep) {
	// If the bit is not enabled, do nothing.
	if (!test) return false;

	G4Track*     trk     = thisStep->GetTrack();
	G4StepPoint* prestep = thisStep->GetPreStepPoint();

	// Bit 0: record particle ID, per-step total energy, and creator process name (if available).
	if (bitIndex == 0) {
		pids.push_back(trk->GetDefinition()->GetPDGEncoding());
		Es.push_back(prestep->GetTotalEnergy());
		if (trk->GetCreatorProcess()) {
			processNames.push_back(trk->GetCreatorProcess()->GetProcessName());
		}
	}
	else if (bitIndex == 1) {
		// Placeholder: record step length and track info.
	}
	else if (bitIndex == 2) {
		// Placeholder: record mother particle track information.
	}
	else if (bitIndex == 3) {
		// Placeholder: record meta information.
	}
	else if (bitIndex == 4) {
		// Placeholder: record optical photon-specific information.
	}
	return true;
}
