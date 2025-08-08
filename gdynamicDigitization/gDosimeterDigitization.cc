/**
 * \file gDosimeterDigitization.cc
 * \brief Implementation of GDosimeterDigitization methods.
 *
 * \mainpage Dosimeter Digitization Module
 *
 * \section intro_sec Introduction
 * This module implements dosimeter digitization for simulation hits. It provides:
 *  - Readout specification definition for dosimeter detectors.
 *  - Conversion of a true hit (GHit) into digitized data (GDigitizedData) including a computed NIEL weight.
 *  - Loading of calibration constants (NIEL factors and particle masses) from external files.
 *  - Linear interpolation of the NIEL factor for a given particle energy.
 *
 * \section details_sec Details
 * The functions implemented herein ensure that:
 *  - Required logger objects are defined via check_if_log_defined().
 *  - The readout specifications are set with a fixed time window, grid start time, and hit bitset.
 *  - The digitization function processes a hit by collecting key variables and computing a NIEL weight
 *    over the hit’s individual steps.
 *  - Calibration data are loaded from text files, using a robust file–reading loop.
 *
 * \author Your Name
 * \date YYYY-MM-DD
 */

#include "ginternalDigitization.h"
#include <fstream>


/**
 * \brief Defines the readout specifications for dosimeter digitization.
 *
 * Sets the electronic readout time-window to 10, the grid start time to 0, and uses the
 * hit bitset "000001" to define what information is stored. A new GReadoutSpecs object is
 * created and assigned to readoutSpecs.
 *
 * \return True if the readout specifications are successfully defined.
 */
bool GDosimeterDigitization::defineReadoutSpecsImpl() {
	double timeWindow    = 10;                  // electronic readout time-window of the detector
	double gridStartTime = 0;                   // defines the windows grid
	auto   hitBitSet     = HitBitSet("000001"); // defines what information to be stored in the hit

	// Create a new GReadoutSpecs object with the specified parameters.
	readoutSpecs = std::make_shared<GReadoutSpecs>(timeWindow, gridStartTime, hitBitSet, log);

	return true;
}

/**
 * \brief Digitizes a hit for dosimeter detectors.
 *
 * This function converts a GHit into a GDigitizedData object. It:
 *  - Checks that required loggers are defined.
 *  - Extracts the first identifier from the hit's identity vector.
 *  - Populates the digitized data with basic hit parameters (hit number, total energy, time, etc.).
 *  - Computes a NIEL weight by iterating over each step of the hit and linearly interpolating
 *    the NIEL factor for valid particle IDs.
 *
 * \param ghit Pointer to the GHit to digitize.
 * \param hitn The hit index.
 * \return Pointer to the newly created GDigitizedData object.
 */
std::unique_ptr<GDigitizedData> GDosimeterDigitization::digitizeHitImpl(GHit* ghit, size_t hitn) {
	// Ensure that required loggers and options are defined.
	check_if_log_defined();

	// ghit->getGID() must have a single entry.
	GIdentifier identity = ghit->getGID().front();

	// Create a new GDigitizedData object for this hit.
	auto gdata = std::make_unique<GDigitizedData>(ghit, data_logger);

	// Include basic hit variables.
	gdata->includeVariable(identity.getName(), identity.getValue());
	gdata->includeVariable("hitn", static_cast<int>(hitn));
	gdata->includeVariable("eTot", ghit->getTotalEnergyDeposited());
	gdata->includeVariable("time", ghit->getAverageTime());

	// Retrieve per-step particle IDs and energies.
	auto pids      = ghit->getPids();
	auto pEnergies = ghit->getEs();

	double nielWeight = 0;
	// Loop over each step.
	for (size_t stepIndex = 0; stepIndex < pids.size(); stepIndex++) {
		// Use absolute value so negative particle IDs (e.g. -11) are handled.
		int pid = std::abs(pids[stepIndex]);

		// Process only for specific particle types.
		if (pid == 11 || pid == 211 || pid == 2212 || pid == 2112) {
			// Compute effective energy by subtracting the particle mass.
			double E = pEnergies[stepIndex] - pMassMeV[pid];
			// Accumulate NIEL weight using linear interpolation of the NIEL factor.
			nielWeight += getNielFactorForParticleAtEnergy(pid, E);
		}
	}

	// Include the computed NIEL weight in the digitized data.
	gdata->includeVariable("nielWeight", nielWeight);

	return gdata;
}

/**
 * \brief Loads digitization constants for dosimeter digitization.
 *
 * Loads NIEL calibration data for various particle types from text files. The files
 * are located in the GEMC data directory under "dosimeterData/Niel/". Each file is expected
 * to contain pairs of values (NIEL factor and corresponding energy threshold). Particle masses
 * for calibration are also loaded.
 *
 * \param runno Run number (unused).
 * \param variation Variation string (unused).
 * \return True if the constants are successfully loaded.
 */
bool GDosimeterDigitization::loadConstantsImpl([[maybe_unused]] int runno, [[maybe_unused]] std::string const& variation) {
	// NIEL Data: map from particle ID (PID) to file name.
	std::map<int, std::string> nielDataFiles;
	nielDataFiles[11]   = "niel_electron.txt";
	nielDataFiles[211]  = "niel_pion.txt";
	nielDataFiles[2112] = "niel_neutron.txt";
	nielDataFiles[2212] = "niel_proton.txt";

	// Construct plugin path from the GEMC environment variable.
	std::filesystem::path gemcRoot = gutilities::gemc_root();
	std::string pluginPath = gemcRoot.string() + "/dosimeterData/";

	// Loop over each particle type and load its NIEL data.
	for (const auto& [pid, filename] : nielDataFiles) {
		std::string dataFileWithPath = pluginPath + "/dosimeterData/Niel/" + filename;

		std::ifstream inputfile(dataFileWithPath);
		if (!inputfile) { log->error(EC__FILENOTFOUND, "Error loading dosimeter data for pid <", pid, "> from file ", dataFileWithPath); }

		log->info(0, " Loading dosimeter data for pid <", pid, "> from file ", dataFileWithPath);

		double p0, p1;
		// Use proper loop condition to read pairs until failure.
		while (inputfile >> p0 >> p1) {
			nielfactorMap[pid].push_back(p0);
			E_nielfactorMap[pid].push_back(p1);
		}
		inputfile.close();
	}

	// Load particle masses (in MeV) for calibration.
	pMassMeV[11]   = 0.510;
	pMassMeV[211]  = 139.570;
	pMassMeV[2112] = 939.565;
	pMassMeV[2212] = 938.272;

	return true;
}

/**
 * \brief Interpolates the NIEL factor for a given particle at a specified energy.
 *
 * For the specified particle (pid) and effective energy (in MeV), this function finds the
 * first threshold where energy is below the recorded value and performs linear interpolation
 * between the adjacent NIEL factors. If the energy is below the first threshold, it returns
 * the first NIEL factor; if above the last threshold, returns the last NIEL factor.
 *
 * \param pid Particle ID.
 * \param energyMeV Effective energy (in MeV) after subtracting particle mass.
 * \return The interpolated NIEL factor.
 */
double GDosimeterDigitization::getNielFactorForParticleAtEnergy(int pid, double energyMeV) {
	// Number of NIEL data points available for this particle.
	auto niel_N = nielfactorMap[pid].size();
	auto j      = niel_N;

	// Find the first index for which the energy is below the threshold.
	for (size_t i = 0; i < niel_N; i++) {
		if (energyMeV < E_nielfactorMap[pid][i]) {
			j = i;
			break;
		}
	}

	double value;
	if (j > 0 && j < niel_N) {
		// Perform linear interpolation between indices j-1 and j.
		auto nielfactor_low  = nielfactorMap[pid][j - 1];
		auto nielfactor_high = nielfactorMap[pid][j];
		auto energy_low      = E_nielfactorMap[pid][j - 1];
		auto energy_high     = E_nielfactorMap[pid][j];
		value                = nielfactor_low + (nielfactor_high - nielfactor_low) / (energy_high - energy_low) * (energyMeV - energy_low);
	}
	else if (j == 0) {
		// Energy is below the first threshold.
		value = nielfactorMap[pid].front();
	}
	else {
		// Energy is above the last threshold.
		value = nielfactorMap[pid].back();
	}

	log->debug(NORMAL, " pid: ", pid, ", j: ", j, ", value: ", value, ", energy: ", energyMeV);

	return value;
}
