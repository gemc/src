#pragma once

#include "gdynamicdigitization.h" // Base class for dynamic digitization.
#include <map>
#include <vector>
#include <string>

/**
 * \class GFluxDigitization
 * \brief Built-in digitization routine for "flux-like" detectors.
 *
 * This digitization routine demonstrates a minimal digitization strategy:
 * it copies a detector identity and basic hit-level observables into the output
 * GDigitizedData.
 *
 * Output variables typically include:
 * - detector identity (the name/value of the first GIdentifier)
 * - hit index ("hitn")
 * - total deposited energy ("totEdep")
 * - average time ("time")
 * - particle id ("pid")
 * - total energy ("totalE")
 *
 * \note
 * The exact naming and presence of variables must match what downstream consumers
 * expect (e.g. converters, output writers, or analysis).
 */
class GFluxDigitization : public GDynamicDigitization
{
public:
	/// Inherit the base constructor (const std::shared_ptr<GOptions>&).
	using GDynamicDigitization::GDynamicDigitization;

	/**
	 * \brief Defines readout specifications for flux digitization.
	 *
	 * This routine sets the electronics readout parameters through a GReadoutSpecs instance.
	 * These settings govern the time-cell binning used when GTouchables are split by time.
	 *
	 * \return true if readout specifications are successfully defined.
	 */
	bool defineReadoutSpecsImpl() override;

	/**
	 * \brief Digitizes a hit for flux detectors.
	 *
	 * This routine converts a GHit into a GDigitizedData record. It is intentionally simple:
	 * it pulls identity and basic observables directly from the hit and stores them as
	 * named variables.
	 *
	 * \param ghit Pointer to the hit to digitize. Ownership stays with the caller.
	 * \param hitn Sequential hit index within the detector collection.
	 * \return A newly allocated digitized record for this hit.
	 */
	std::unique_ptr<GDigitizedData> digitizeHitImpl(GHit* ghit, size_t hitn) override;
};
