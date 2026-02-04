/**
 * \file gDosimeterDigitization.cc
 * \brief Implementation of GDosimeterDigitization.
 *
 * This translation unit provides the implementation of the internal dosimeter digitization
 * plugin. Public API documentation for GDosimeterDigitization lives in
 * ginternalDigitization.h.  This file intentionally keeps only local, non-Doxygen
 * comments to avoid duplicating the header documentation.
 */

#include "ginternalDigitization.h"
#include <fstream>

// See header for API docs.
bool GDosimeterDigitization::defineReadoutSpecsImpl() {
	// Time window is the width of one electronics time cell (unit follows the convention
	// used by the rest of the digitization chain; typically ns).
	double timeWindow    = 10;                  // electronic readout time-window of the detector
	double gridStartTime = 0;                   // defines the windows grid
	auto   hitBitSet     = HitBitSet("000001"); // defines what information to be stored in the hit

	// Readout specs are immutable after initialization and shared by all processed hits.
	readoutSpecs = std::make_shared<GReadoutSpecs>(timeWindow, gridStartTime, hitBitSet, log);

	return true;
}

// See header for API docs.
std::unique_ptr<GDigitizedData> GDosimeterDigitization::digitizeHitImpl(GHit* ghit, [[maybe_unused]] size_t hitn) {
	check_if_log_defined();

	// Expected to be a single-identity detector: take the first identity entry.
	GIdentifier identity = ghit->getGID().front();

	auto gdata = std::make_unique<GDigitizedData>(gopts, ghit);

	// Store the detector identity and the total deposited energy.
	gdata->includeVariable(identity.getName(), identity.getValue());
	gdata->includeVariable("eTot", ghit->getTotalEnergyDeposited());

	// Per-step information used to build the NIEL-weight.
	auto pids      = ghit->getPids();
	auto pEnergies = ghit->getEs();

	double nielWeight = 0;

	// Accumulate NIEL factor step-by-step. This treats each step independently and sums
	// the interpolated factors for supported particle species.
	for (size_t stepIndex = 0; stepIndex < pids.size(); stepIndex++) {
		// Use absolute PID so negative particle IDs (e.g. -11) are handled.
		int pid = std::abs(pids[stepIndex]);

		// Only a few particle types are supported by the calibration data files.
		if (pid == 11 || pid == 211 || pid == 2212 || pid == 2112) {
			// Convert from total energy to kinetic-like quantity used by the NIEL tables
			// by subtracting the particle rest mass (in MeV).
			double E   = pEnergies[stepIndex] - pMassMeV[pid];
			nielWeight += getNielFactorForParticleAtEnergy(pid, E);
		}
	}

	gdata->includeVariable("nielWeight", nielWeight);

	return gdata;
}

// See header for API docs.
bool GDosimeterDigitization::loadConstantsImpl([[maybe_unused]] int                runno,
                                               [[maybe_unused]] std::string const& variation) {
	// Map from particle id to the calibration filename (text, 2 columns).
	std::map<int, std::string> nielDataFiles;
	nielDataFiles[11]   = "niel_electron.txt";
	nielDataFiles[211]  = "niel_pion.txt";
	nielDataFiles[2112] = "niel_neutron.txt";
	nielDataFiles[2212] = "niel_proton.txt";

	// GEMC installation root used to locate plugin data.
	std::filesystem::path gemcRoot = gutilities::gemc_root();

	for (const auto& [pid, filename] : nielDataFiles) {
		std::string dataFileWithPath = gemcRoot.string() + "/dosimeterData" + "/Niel/" + filename;

		std::ifstream inputfile(dataFileWithPath);
		if (!inputfile) {
			// On Linux, tests may run from the build directory, where plugin data lives under
			// gdynamicDigitization/...
			dataFileWithPath = gemcRoot.string() + "/gdynamicDigitization/dosimeterData" + "/Niel/" + filename;
			inputfile.open(dataFileWithPath);
			if (!inputfile) {
				log->error(EC__FILENOTFOUND, "Error loading dosimeter data for pid <", pid, "> from file ",
				           dataFileWithPath);
			}
		}

		log->info(0, " Loading dosimeter data for pid <", pid, "> from file ", dataFileWithPath);

		// Expected file format: repeated pairs (factor, energyMeV).
		double p0, p1;
		while (inputfile >> p0 >> p1) {
			nielfactorMap[pid].push_back(p0);
			E_nielfactorMap[pid].push_back(p1);
		}
		inputfile.close();
	}

	// Particle rest masses used by the interpolation routine (MeV).
	pMassMeV[11]   = 0.510;
	pMassMeV[211]  = 139.570;
	pMassMeV[2112] = 939.565;
	pMassMeV[2212] = 938.272;

	return true;
}

// See header for API docs.
double GDosimeterDigitization::getNielFactorForParticleAtEnergy(int pid, double energyMeV) {
	const auto niel_N = nielfactorMap[pid].size();

	// Guard against missing/empty calibration data.
	// (If this happens, it indicates loadConstantsImpl did not populate the maps as expected.)
	if (niel_N == 0) {
		log->error(EC__FILENOTFOUND, "NIEL tables are empty for pid <", pid, ">. Did loadConstantsImpl fail?");
		return 0.0;
	}

	// j is the first index for which energyMeV < E_table[j]. Initialize to "past the end".
	size_t j = niel_N;

	for (size_t i = 0; i < niel_N; i++) {
		if (energyMeV < E_nielfactorMap[pid][i]) {
			j = i;
			break;
		}
	}

	double value = 0.0;

	if (j > 0 && j < niel_N) {
		// Linear interpolation between (j-1) and j.
		const auto nielfactor_low  = nielfactorMap[pid][j - 1];
		const auto nielfactor_high = nielfactorMap[pid][j];
		const auto energy_low      = E_nielfactorMap[pid][j - 1];
		const auto energy_high     = E_nielfactorMap[pid][j];

		value = nielfactor_low +
			(nielfactor_high - nielfactor_low) / (energy_high - energy_low) * (energyMeV - energy_low);
	}
	else if (j == 0) {
		// energy below the first threshold: clamp to first value.
		value = nielfactorMap[pid].front();
	}
	else {
		// energy beyond the last threshold: clamp to last value.
		value = nielfactorMap[pid].back();
	}

	log->debug(NORMAL, " pid: ", pid, ", j: ", j, ", value: ", value, ", energy: ", energyMeV);

	return value;
}
