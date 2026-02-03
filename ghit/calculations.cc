// ghit
#include "ghit.h"

// glibrary
#include "gutsConventions.h"

// See header for API docs.

/**
 * Implementation notes (non-Doxygen):
 * - Derived quantities use a lazy-cache model (computed on first access).
 * - Energy-weighted averages are used when total deposited energy is non-zero.
 * - When total energy is zero, fall back to simple arithmetic averaging.
 */

void GHit::calculateInfosForBit(int bit) {
	// Bit 0: always present - calculate total energy, average time, and average positions.
	if (bit == 0) {
		// Ensure total energy is available (and cached).
		double tote = getTotalEnergyDeposited();

		double avgx  = 0, avgy  = 0, avgz  = 0;
		double avglx = 0, avgly = 0, avglz = 0;
		averageTime  = 0;

		auto nsteps = edeps.size();
		for (size_t s = 0; s < nsteps; s++) {
			// Energy-weighted average when possible; otherwise arithmetic average.
			if (tote > 0) {
				averageTime += times[s] * edeps[s] / tote;
				avgx        += globalPositions[s].getX() * edeps[s] / tote;
				avgy        += globalPositions[s].getY() * edeps[s] / tote;
				avgz        += globalPositions[s].getZ() * edeps[s] / tote;
				avglx       += localPositions[s].getX() * edeps[s] / tote;
				avgly       += localPositions[s].getY() * edeps[s] / tote;
				avglz       += localPositions[s].getZ() * edeps[s] / tote;
			}
			else {
				// Fallback to simple averaging if no energy is deposited.
				averageTime += times[s] / nsteps;
				avgx        += globalPositions[s].getX() / nsteps;
				avgy        += globalPositions[s].getY() / nsteps;
				avgz        += globalPositions[s].getZ() / nsteps;
				avglx       += localPositions[s].getX() / nsteps;
				avgly       += localPositions[s].getY() / nsteps;
				avglz       += localPositions[s].getZ() / nsteps;
			}
		}
		avgGlobalPosition = G4ThreeVector(avgx, avgy, avgz);
		avgLocalPosition  = G4ThreeVector(avglx, avgly, avglz);

		// Use the first process name if available as a representative label.
		if (!processNames.empty()) {
			processName = processNames.front();
		}
	}
	// Future extensions for bits 1-4 can be added below.
	else if (bit == 1) {
		// Placeholder for step length and track information.
	}
	else if (bit == 2) {
		// Placeholder for mother particle tracks information.
	}
	else if (bit == 3) {
		// Placeholder for meta information.
	}
	else if (bit == 4) {
		// Placeholder for optical photon specific information.
	}
}

double GHit::getTotalEnergyDeposited() {
	if (!totalEnergyDeposited.has_value()) {
		// Cache the sum of per-step energy depositions.
		double sum = 0;
		for (const auto& ei : edeps) {
			sum += ei;
		}
		totalEnergyDeposited = sum;
	}
	return totalEnergyDeposited.value();
}

double GHit::getAverageTime() {
	if (averageTime == UNINITIALIZEDNUMBERQUANTITY) {
		double tote = getTotalEnergyDeposited();

		averageTime = 0;
		auto nsteps = edeps.size();
		for (size_t s = 0; s < nsteps; s++) {
			// Keep the existing behavior: compare against totalEnergyDeposited (optional) as written.
			// This block computes an energy-weighted average when possible.
			if (totalEnergyDeposited > 0) {
				averageTime += times[s] * edeps[s] / tote;
			}
			else {
				averageTime += times[s] / nsteps;
			}
		}
	}

	return averageTime;
}

G4ThreeVector GHit::getAvgGlobaPosition() {
	if (avgGlobalPosition.getX() == UNINITIALIZEDNUMBERQUANTITY && avgGlobalPosition.getY() ==
		UNINITIALIZEDNUMBERQUANTITY) {
		double tote = getTotalEnergyDeposited();

		double avgx = 0, avgy = 0, avgz = 0;

		auto nsteps = edeps.size();
		for (size_t s = 0; s < nsteps; s++) {
			// Energy-weighted average when possible; otherwise arithmetic average.
			if (totalEnergyDeposited > 0) {
				avgx += globalPositions[s].getX() * edeps[s] / tote;
				avgy += globalPositions[s].getY() * edeps[s] / tote;
				avgz += globalPositions[s].getZ() * edeps[s] / tote;
			}
			else {
				// Preserve existing behavior (note: this branch also increments averageTime as written).
				averageTime += times[s] / nsteps;
				avgx        += globalPositions[s].getX() / nsteps;
				avgy        += globalPositions[s].getY() / nsteps;
				avgz        += globalPositions[s].getZ() / nsteps;
			}
		}
		avgGlobalPosition = G4ThreeVector(avgx, avgy, avgz);
	}

	return avgGlobalPosition;
}

G4ThreeVector GHit::getAvgLocalPosition() {
	if (avgLocalPosition.getX() == UNINITIALIZEDNUMBERQUANTITY && avgLocalPosition.getY() ==
		UNINITIALIZEDNUMBERQUANTITY) {
		double tote = getTotalEnergyDeposited();

		double avgx = 0, avgy = 0, avgz = 0;

		auto nsteps = edeps.size();
		for (size_t s = 0; s < nsteps; s++) {
			// Energy-weighted average when possible; otherwise arithmetic average.
			if (totalEnergyDeposited > 0) {
				avgx += localPositions[s].getX() * edeps[s] / tote;
				avgy += localPositions[s].getY() * edeps[s] / tote;
				avgz += localPositions[s].getZ() * edeps[s] / tote;
			}
			else {
				// Preserve existing behavior (note: this branch also increments averageTime as written).
				averageTime += times[s] / nsteps;
				avgx        += localPositions[s].getX() / nsteps;
				avgy        += localPositions[s].getY() / nsteps;
				avgz        += localPositions[s].getZ() / nsteps;
			}
		}
		avgLocalPosition = G4ThreeVector(avgx, avgy, avgz);
	}
	return avgLocalPosition;
}
