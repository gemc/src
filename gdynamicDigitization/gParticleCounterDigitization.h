#pragma once

#include "gdynamicdigitization.h" // Base class for dynamic digitization.
#include <map>
#include <vector>
#include <string>

/**
 * \class GParticleCounterDigitization
 * \brief Built-in digitization routine for particle counters.
 *
 * Similar to GFluxDigitization, but typically used for detectors whose main purpose is
 * to count/flag particle passage rather than model electronics shaping.
 *
 * \note
 * This routine uses a different hit bitset compared to flux/dosimeter routines.
 * The bitset controls which true-hit information is computed/available at the hit level.
 */
class GParticleCounterDigitization : public GDynamicDigitization
{
public:
	/// Inherit the base constructor (const std::shared_ptr<GOptions>&).
	using GDynamicDigitization::GDynamicDigitization;

	/**
	 * \brief Defines readout specifications for particle counter digitization.
	 *
	 * \return true if readout specifications are successfully defined.
	 */
	bool defineReadoutSpecsImpl() override;

	/**
	 * \brief Digitizes a hit for particle counters.
	 *
	 * Copies identity and basic observables from the hit into the digitized record.
	 *
	 * \param ghit Pointer to the hit to digitize. Ownership stays with the caller.
	 * \param hitn Sequential hit index within the detector collection.
	 * \return A newly allocated digitized record for this hit.
	 */
	std::unique_ptr<GDigitizedData> digitizeHitImpl(GHit* ghit, size_t hitn) override;
};
