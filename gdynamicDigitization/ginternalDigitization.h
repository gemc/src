#pragma once

/**
 * \file ginternalDigitization.h
 * \brief Internal digitization plugins shipped with the gdynamic digitization library.
 *
 * This header declares a small set of built-in digitization routines that can be used
 * as plugins. These classes derive from GDynamicDigitization and implement:
 * - \ref GDynamicDigitization::defineReadoutSpecs "defineReadoutSpecs()"
 * - \ref GDynamicDigitization::digitizeHit "digitizeHit()"
 *
 * GDosimeterDigitization additionally implements:
 * - \ref GDynamicDigitization::loadConstants "loadConstants()"
 *
 * \note
 * The authoritative API documentation for these plugins is in this header.
 * Their .cc files contain only implementation details and minimal, non-Doxygen comments.
 */

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

/**
 * \class GDosimeterDigitization
 * \brief Built-in digitization routine for dosimeters with NIEL weighting.
 *
 * This digitization routine computes a NIEL-weight-like quantity by iterating over the
 * per-step particle IDs and energies stored in the input hit, and summing an interpolated
 * NIEL factor from calibration tables on disk.
 *
 * The calibration tables are loaded by \ref GDynamicDigitization::loadConstants "loadConstants()"
 * and stored in internal maps keyed by particle id.
 *
 * Digitized output variables include (at minimum):
 * - detector identity (the name/value of the first GIdentifier)
 * - total deposited energy ("eTot")
 * - computed NIEL sum ("nielWeight")
 *
 * \warning
 * This routine assumes that the dosimeter data files exist in the expected installation
 * paths. If a file cannot be opened, an error is logged.
 */
class GDosimeterDigitization : public GDynamicDigitization
{
public:
	/// Inherit the base constructor (const std::shared_ptr<GOptions>&).
	using GDynamicDigitization::GDynamicDigitization;

	/**
	 * \brief Defines readout specifications for dosimeter digitization.
	 *
	 * \return true if readout specifications are successfully defined.
	 */
	bool defineReadoutSpecsImpl() override;

	/**
	 * \brief Digitizes a hit for dosimeter detectors.
	 *
	 * This routine:
	 * 1. Stores identity and hit-level totals (energy).
	 * 2. Walks the per-step arrays (pid and energy) stored in the hit.
	 * 3. For supported particle species, computes an effective energy (MeV) as:
	 *    effectiveEnergy = stepEnergy - particleRestMass
	 * 4. Sums the interpolated NIEL factor evaluated at that effective energy.
	 *
	 * \param ghit Pointer to the hit to digitize. Ownership stays with the caller.
	 * \param hitn Sequential hit index within the detector collection (unused).
	 * \return A newly allocated digitized record for this hit.
	 */
	std::unique_ptr<GDigitizedData> digitizeHitImpl(GHit* ghit, size_t hitn) override;

	/**
	 * \brief Loads digitization constants for dosimeter digitization.
	 *
	 * This routine loads:
	 * - NIEL factor tables from text files (two columns: factor and energy in MeV).
	 * - Particle rest masses in MeV used to compute effective energy.
	 *
	 * File locations are resolved relative to the GEMC installation root.
	 *
	 * \param runno Run number (unused by this routine).
	 * \param variation Variation string (unused by this routine).
	 * \return true if constants are successfully loaded (file failures are logged).
	 */
	bool loadConstantsImpl(int runno, std::string const& variation) override;

private:
	/**
	 * \brief NIEL factor values indexed by particle id.
	 *
	 * For each particle species, the values in this vector correspond 1-to-1 with the
	 * energies stored in \c E_nielfactorMap for the same particle id.
	 */
	std::map<int, std::vector<double>> nielfactorMap;

	/**
	 * \brief Energy grid (MeV) for NIEL factor interpolation, indexed by particle id.
	 *
	 * These energies are interpreted as the independent variable of the calibration table.
	 * Interpolation is performed between adjacent grid points.
	 */
	std::map<int, std::vector<double>> E_nielfactorMap;

	/**
	 * \brief Particle rest masses (MeV) used to compute effective energy for the tables.
	 *
	 * Keys are particle ids. Only ids supported by the calibration tables are expected
	 * to be present.
	 */
	std::map<int, double> pMassMeV;

	/**
	 * \brief Interpolates the NIEL factor for a given particle at a specified energy.
	 *
	 * The routine clamps outside the table domain:
	 * - for energies below the first grid point, returns the first factor
	 * - for energies above the last grid point, returns the last factor
	 *
	 * \param pid Particle id key into the calibration tables.
	 * \param energyMeV Effective energy in MeV (typically stepEnergy - restMass).
	 * \return Interpolated (or clamped) NIEL factor.
	 */
	double getNielFactorForParticleAtEnergy(int pid, double energyMeV);
};
