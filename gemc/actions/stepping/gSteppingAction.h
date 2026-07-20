#pragma once

// Geant4
#include "G4UserSteppingAction.hh"

/**
 * \brief Applies per-step track guards for one Geant4 worker.
 *
 * Replicates the GEMC2 MSteppingAction protections:
 * - optical photons are killed after \c MAX_OPTICAL_PHOTON_STEPS steps: photons trapped by total
 *   internal reflection in volumes with no absorption length would otherwise bounce forever
 * - any track is killed after \c MAX_TRACK_STEPS steps (e.g. stuck in a magnetic field loop)
 * - tracks touching the Kryptonite material are killed
 */
class GSteppingAction : public G4UserSteppingAction
{
public:
	void UserSteppingAction(const G4Step* step) override;
};
