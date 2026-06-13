#pragma once

#include "gFluxDigitization.h"

/**
 * \class GPhotonDetectorDigitization
 * \brief Built-in digitization routine for optical-photon flux detectors.
 *
 * This routine records the same output fields as \c flux, but accepts only Geant4
 * optical-photon steps. Optical photons are recorded even when they deposit zero
 * energy, independent of the global \c recordZeroEdep switch.
 */
class GPhotonDetectorDigitization : public GFluxDigitization
{
public:
	/// Inherit the base constructor (const std::shared_ptr<GOptions>&).
	using GFluxDigitization::GFluxDigitization;

	/// Keep the base's energy-only overload visible alongside the override below,
	/// so it is not hidden by name lookup (-Woverloaded-virtual).
	using GFluxDigitization::decisionToSkipHit;

	/**
	 * \brief Keeps only optical-photon steps.
	 *
	 * \param energy Total deposited energy for the step.
	 * \param thisStep Geant4 step being processed.
	 * \return true when the step should be skipped.
	 */
	bool decisionToSkipHit(double energy, const G4Step* thisStep) override;
};
