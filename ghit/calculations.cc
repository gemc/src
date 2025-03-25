// ghit
#include "ghit.h"

// glibrary
#include "gutsConventions.h"


/**
 * \brief Calculates averaged hit information for the specified bit.
 *
 * For bit 0, it calculates the total energy deposited, average time, and weighted average positions.
 * The computation uses energy deposition as weights. For other bits, placeholders exist for future extensions.
 * Notice: if the energy deposited is very low (~50eV), the rounding error on the average calculations could be up to 10^-3.
 *
 * \param bit The bit index for which to calculate the information.
 */
void GHit::calculateInfosForBit(int bit) {
	// Bit 0: always present - calculate total energy, average time, and average positions.
	if (bit == 0) {
		double tote = getTotalEnergyDeposited();

		float avgx = 0, avgy = 0, avgz = 0;
		float avglx = 0, avgly = 0, avglz = 0;
		averageTime = 0;

		auto nsteps = edeps.size();
		for (size_t s = 0; s < nsteps; s++) {
			// Energy-weighted average.
			if (tote > 0) {
				averageTime += times[s] * edeps[s] / tote;
				avgx += globalPositions[s].getX() * edeps[s] / tote;
				avgy += globalPositions[s].getY() * edeps[s] / tote;
				avgz += globalPositions[s].getZ() * edeps[s] / tote;
				avglx += localPositions[s].getX() * edeps[s] / tote;
				avgly += localPositions[s].getY() * edeps[s] / tote;
				avglz += localPositions[s].getZ() * edeps[s] / tote;
			} else { // Fallback to simple averaging if no energy is deposited.
				averageTime += times[s] / nsteps;
				avgx += globalPositions[s].getX() / nsteps;
				avgy += globalPositions[s].getY() / nsteps;
				avgz += globalPositions[s].getZ() / nsteps;
				avglx += localPositions[s].getX() / nsteps;
				avgly += localPositions[s].getY() / nsteps;
				avglz += localPositions[s].getZ() / nsteps;
			}
		}
		avgGlobalPosition = G4ThreeVector(avgx, avgy, avgz);
		avgLocalPosition = G4ThreeVector(avglx, avgly, avglz);

		// Use the first process name if available.
		if (!processNames.empty()) {
			processName = processNames.front();
		}
	}
		// Future extensions for bits 1-4 can be added below.
	else if (bit == 1) {
		// Placeholder for step length and track information.
	} else if (bit == 2) {
		// Placeholder for mother particle tracks information.
	} else if (bit == 3) {
		// Placeholder for meta information.
	} else if (bit == 4) {
		// Placeholder for optical photon specific information.
	}
}


/**
 * \brief Computes the total energy deposited.
 *
 * Sums the energy depositions over all steps.
 *
 * \return The total energy deposited.
 */
// Then in getTotalEnergyDeposited():
float GHit::getTotalEnergyDeposited() {
	if (!totalEnergyDeposited.has_value()) {
		float sum = 0;
		for (const auto &ei: edeps) {
			sum += ei;
		}
		totalEnergyDeposited = sum;
	}
	return totalEnergyDeposited.value();
}

float GHit::getAverageTime() {
	if (averageTime == UNINITIALIZEDNUMBERQUANTITY) {

		double tote = getTotalEnergyDeposited();


		averageTime = 0;
		auto nsteps = edeps.size();
		for (size_t s = 0; s < nsteps; s++) {
			if (totalEnergyDeposited > 0) {
				averageTime += times[s] * edeps[s] / tote;
			} else {
				averageTime += times[s] / nsteps;
			}
		}

	}

	return averageTime;
}


/**
 * \brief Computes the average global position of the hit.
 *
 * Uses energy-weighted averaging if energy deposition is nonzero.
 * \return The averaged global position as a G4ThreeVector.
 */
G4ThreeVector GHit::getAvgGlobaPosition() {

	if (avgGlobalPosition.getX() == UNINITIALIZEDNUMBERQUANTITY && avgGlobalPosition.getY() == UNINITIALIZEDNUMBERQUANTITY) {

		double tote = getTotalEnergyDeposited();

		float avgx = 0, avgy = 0, avgz = 0;

		auto nsteps = edeps.size();
		for (size_t s = 0; s < nsteps; s++) {
			if (totalEnergyDeposited > 0) {
				avgx += globalPositions[s].getX() * edeps[s] / tote;
				avgy += globalPositions[s].getY() * edeps[s] / tote;
				avgz += globalPositions[s].getZ() * edeps[s] / tote;
			} else {
				averageTime += times[s] / nsteps;
				avgx += globalPositions[s].getX() / nsteps;
				avgy += globalPositions[s].getY() / nsteps;
				avgz += globalPositions[s].getZ() / nsteps;
			}
		}
		avgGlobalPosition = G4ThreeVector(avgx, avgy, avgz);
	}

	return avgGlobalPosition;
}

/**
 * \brief Computes the average local position of the hit.
 *
 * Uses energy-weighted averaging if available.
 * \return The averaged local position as a G4ThreeVector.
 */
G4ThreeVector GHit::getAvgLocalPosition() {

	if (avgLocalPosition.getX() == UNINITIALIZEDNUMBERQUANTITY && avgLocalPosition.getY() == UNINITIALIZEDNUMBERQUANTITY) {

		double tote = getTotalEnergyDeposited();

		float avgx = 0, avgy = 0, avgz = 0;

		auto nsteps = edeps.size();
		for (size_t s = 0; s < nsteps; s++) {
			if (totalEnergyDeposited > 0) {
				avgx += localPositions[s].getX() * edeps[s] / tote;
				avgy += localPositions[s].getY() * edeps[s] / tote;
				avgz += localPositions[s].getZ() * edeps[s] / tote;
			} else {
				averageTime += times[s] / nsteps;
				avgx += localPositions[s].getX() / nsteps;
				avgy += localPositions[s].getY() / nsteps;
				avgz += localPositions[s].getZ() / nsteps;
			}
		}
		avgLocalPosition = G4ThreeVector(avgx, avgy, avgz);

	}
	return avgLocalPosition;
}
