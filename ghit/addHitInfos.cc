// ghit
#include "ghit.h"

// geant4
#include "G4TouchableHistory.hh"
#include "G4VProcess.hh"

/**
 * \brief Adds hit information based on a HitBitSet.
 *
 * This method extracts common hit data (positions, energy deposit, time)
 * and then iterates over the bits in the HitBitSet to add additional information.
 *
 * \param hbs The HitBitSet that selects which additional information to record.
 * \param thisStep The current G4Step from which hit data is extracted.
 */
void GHit::addHitInfosForBitset(const HitBitSet hbs, const G4Step *thisStep) {
	G4StepPoint *poststep = thisStep->GetPostStepPoint();
	// Get global position and transform to local coordinates.
	G4ThreeVector xyz = poststep->GetPosition();
	G4ThreeVector xyzL = poststep->GetTouchableHandle()->GetHistory()->GetTopTransform().TransformPoint(xyz);
	globalPositions.push_back(xyz);
	localPositions.push_back(xyzL);

	// Retrieve energy deposition and time information.
	float edep = (thisStep->GetTotalEnergyDeposit()) * (gtouchable->getEnergyMultiplier());
	float time = poststep->GetGlobalTime();
	edeps.push_back(edep);
	times.push_back(time);

	// Iterate over each bit and call the helper method to add extra info.
	for (size_t hbIndex = 0; hbIndex < hbs.size(); hbIndex++) {
		addHitInfosForBitIndex(hbIndex, hbs.test(hbIndex), thisStep);
	}
}

/**
 * \brief Helper function to add extra hit information for a given bit index.
 *
 * Checks if the bit is set. For bit 0, records particle IDs, energy,
 * and process name. Future bits (1-4) are placeholders.
 *
 * \param bitIndex The index of the bit.
 * \param test Boolean indicating whether the bit is set.
 * \param thisStep The G4Step from which information is extracted.
 * \return True if extra information was added.
 */
bool GHit::addHitInfosForBitIndex(size_t bitIndex, const bool test, const G4Step *thisStep) {

	if (!test) return false; // If the bit is not set, do nothing.

	G4Track *trk = thisStep->GetTrack();
	G4StepPoint *prestep = thisStep->GetPreStepPoint();

	// For bit 0: record particle ID, energy, and process name.
	if (bitIndex == 0) {
		pids.push_back(trk->GetDefinition()->GetPDGEncoding());
		Es.push_back(prestep->GetTotalEnergy());
		if (trk->GetCreatorProcess()) {
			processNames.push_back(trk->GetCreatorProcess()->GetProcessName());
		}
	} else if (bitIndex == 1) {
		// Placeholder: record step length and track info.
	} else if (bitIndex == 2) {
		// Placeholder: record mother particle track information.
	} else if (bitIndex == 3) {
		// Placeholder: record meta information.
	} else if (bitIndex == 4) {
		// Placeholder: record optical photon-specific information.
	}
	return true;
}
