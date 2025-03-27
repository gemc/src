#ifndef GINTERNALDIGITIZATION
#define GINTERNALDIGITIZATION 1

/**
 * \file ginternalDigitization.h
 * \brief Declares internal digitization classes for various detector types.
 *
 * \mainpage Internal Digitization Module
 *
 * \section intro_sec Introduction
 * This module defines classes that implement dynamic digitization for various
 * detector systems in the simulation. Each class inherits from GDynamicDigitization
 * and provides a concrete implementation for defining readout specifications and
 * digitizing hits.
 *
 * \section classes_sec Classes
 * - **GFluxDigitization**: Digitizes hits for flux detectors.
 * - **GParticleCounterDigitization**: Digitizes hits for particle counter detectors.
 * - **GDosimeterDigitization**: Digitizes hits for dosimeter detectors and loads calibration constants.
 *
 * \section usage_sec Usage
 * Instantiate one of these classes via your dynamic plugin system. Each derived class
 * must implement defineReadoutSpecs() and digitizeHit(). Additionally, GDosimeterDigitization
 * implements loadConstants() for calibration.
 *
 * \author Your Name
 * \date YYYY-MM-DD
 */

#include "gdynamicdigitization.h"  // Base class for dynamic digitization.
#include <map>
#include <vector>
#include <string>

/// Class for flux detector digitization. Inherits from GDynamicDigitization.
class GFluxDigitization : public GDynamicDigitization {
public:
	/**
	 * \brief Defines readout specifications for flux digitization.
	 *
	 * Initializes readout parameters (time window, grid start time, and hit bitset)
	 * specific to flux detectors.
	 *
	 * \return True if readout specifications are successfully defined.
	 */
	bool defineReadoutSpecsImpl() override;

	/**
	 * \brief Digitizes a hit for flux detectors.
	 *
	 * Processes the hit data and returns a pointer to the resulting digitized data.
	 *
	 * \param ghit Pointer to the hit (GHit) to be digitized.
	 * \param hitn The hit index.
	 * \return Pointer to a newly created GDigitizedData object.
	 */
	GDigitizedData* digitizeHitImpl(GHit *ghit, size_t hitn) override;
};

/// Class for particle counter digitization. Inherits from GDynamicDigitization.
class GParticleCounterDigitization : public GDynamicDigitization {
public:
	/**
	 * \brief Defines readout specifications for particle counter digitization.
	 *
	 * Sets up readout parameters specific to particle counter detectors.
	 *
	 * \return True if readout specifications are successfully defined.
	 */
	bool defineReadoutSpecsImpl() override;

	/**
	 * \brief Digitizes a hit for particle counters.
	 *
	 * Processes the hit data and returns a pointer to the resulting digitized data.
	 *
	 * \param ghit Pointer to the hit (GHit) to be digitized.
	 * \param hitn The hit index.
	 * \return Pointer to a newly created GDigitizedData object.
	 */
	GDigitizedData* digitizeHitImpl(GHit *ghit, size_t hitn) override;
};

/// Class for dosimeter digitization. Inherits from GDynamicDigitization.
class GDosimeterDigitization : public GDynamicDigitization {
public:
	/**
	 * \brief Defines readout specifications for dosimeter digitization.
	 *
	 * Initializes readout parameters specific to dosimeter detectors.
	 *
	 * \return True if readout specifications are successfully defined.
	 */
	bool defineReadoutSpecsImpl() override;

	/**
	 * \brief Digitizes a hit for dosimeter detectors.
	 *
	 * Processes the hit data and returns a pointer to the resulting digitized data.
	 *
	 * \param ghit Pointer to the hit (GHit) to be digitized.
	 * \param hitn The hit index.
	 * \return Pointer to a newly created GDigitizedData object.
	 */
	GDigitizedData* digitizeHitImpl(GHit *ghit, size_t hitn) override;

	/**
	 * \brief Loads digitization constants for dosimeter digitization.
	 *
	 * Loads calibration constants (such as NIEL factors) for different particle types.
	 *
	 * \param runno The run number.
	 * \param variation A variation string (e.g. configuration identifier).
	 * \return True if the constants are successfully loaded.
	 */
	bool loadConstantsImpl(int runno, std::string const &variation) override;

private:
	// Calibration maps (keys are particle IDs).
	// Currently:
	// 11 (electrons), 211 (pions), 2112 (neutrons), 2212 (protons)
	std::map<int, std::vector<double>> nielfactorMap;
	std::map<int, std::vector<double>> E_nielfactorMap;
	std::map<int, double> pMassMeV; ///< Particle mass in MeV for different particle IDs.

	/**
	 * \brief Retrieves the NIEL factor for a given particle at a specified energy.
	 *
	 * Computes or retrieves the NIEL factor based on the particle ID and energy.
	 *
	 * \param pid Particle ID.
	 * \param energy The energy value.
	 * \return The NIEL factor as a double.
	 */
	float getNielFactorForParticleAtEnergy(int pid, float energy);
};

#endif
