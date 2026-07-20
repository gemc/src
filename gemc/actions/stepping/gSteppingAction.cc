#include "gSteppingAction.h"
#include "../gactionConventions.h"

// Geant4
#include "G4OpticalPhoton.hh"
#include "G4Step.hh"
#include "G4Track.hh"

void GSteppingAction::UserSteppingAction(const G4Step* step) {
	G4Track* track = step->GetTrack();

	// Optical photons rarely take more than ~20 steps in Cherenkov detectors; a photon
	// exceeding this is trapped (e.g. total internal reflection in a volume with no
	// absorption length) and would step forever.
	if (track->GetDefinition() == G4OpticalPhoton::OpticalPhotonDefinition() &&
		track->GetCurrentStepNumber() > MAX_OPTICAL_PHOTON_STEPS) {
		track->SetTrackStatus(fStopAndKill);
		return;
	}

	// A track can get stuck in a magnetic field stepping loop.
	if (track->GetCurrentStepNumber() > MAX_TRACK_STEPS) {
		track->SetTrackStatus(fStopAndKill);
		return;
	}

	// Anything touching Kryptonite is killed.
	static const G4String kryptonite = KRYPTONITE_KILL_MATERIAL;
	if (track->GetMaterial()->GetName() == kryptonite) {
		track->SetTrackStatus(fStopAndKill);
	}
}
