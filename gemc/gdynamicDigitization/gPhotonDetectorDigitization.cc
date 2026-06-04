#include "gPhotonDetectorDigitization.h"

// geant4
#include "G4OpticalPhoton.hh"
#include "G4Step.hh"

bool GPhotonDetectorDigitization::decisionToSkipHit(double energy, const G4Step* thisStep) {
	if (thisStep == nullptr || thisStep->GetTrack() == nullptr) {
		return true;
	}

	const auto* particle = thisStep->GetTrack()->GetDefinition();
	if (particle != G4OpticalPhoton::OpticalPhotonDefinition()) {
		return true;
	}

	if (energy == 0) {
		return false;
	}

	return GDynamicDigitization::decisionToSkipHit(energy);
}
