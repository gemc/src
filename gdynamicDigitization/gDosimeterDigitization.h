#pragma once

#include "gdynamicdigitization.h" // Base class for dynamic digitization.
#include <map>
#include <vector>
#include <string>

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
