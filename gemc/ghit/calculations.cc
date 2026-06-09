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

void GHit::calculateInfos() {
	double tote = getTotalEnergyDeposited();

	double avgx  = 0, avgy  = 0, avgz  = 0;
	double avglx = 0, avgly = 0, avglz = 0;
	averageTime  = 0;

	auto nsteps_ = edeps.size();
	for (size_t s = 0; s < nsteps_; s++) {
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
			averageTime += times[s] / nsteps_;
			avgx        += globalPositions[s].getX() / nsteps_;
			avgy        += globalPositions[s].getY() / nsteps_;
			avgz        += globalPositions[s].getZ() / nsteps_;
			avglx       += localPositions[s].getX() / nsteps_;
			avgly       += localPositions[s].getY() / nsteps_;
			avglz       += localPositions[s].getZ() / nsteps_;
		}
	}
	avgGlobalPosition = G4ThreeVector(avgx, avgy, avgz);
	avgLocalPosition  = G4ThreeVector(avglx, avgly, avglz);

	// Use the first process name as the representative label for this hit.
	if (!processNames.empty()) {
		processName = processNames.front();
	}
}

double GHit::getTotalEnergyDeposited() {
	if (!totalEnergyDeposited.has_value()) {
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
		auto nsteps_ = edeps.size();
		for (size_t s = 0; s < nsteps_; s++) {
			if (totalEnergyDeposited > 0) {
				averageTime += times[s] * edeps[s] / tote;
			}
			else {
				averageTime += times[s] / nsteps_;
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

		auto nsteps_ = edeps.size();
		for (size_t s = 0; s < nsteps_; s++) {
			if (totalEnergyDeposited > 0) {
				avgx += globalPositions[s].getX() * edeps[s] / tote;
				avgy += globalPositions[s].getY() * edeps[s] / tote;
				avgz += globalPositions[s].getZ() * edeps[s] / tote;
			}
			else {
				averageTime += times[s] / nsteps_;
				avgx        += globalPositions[s].getX() / nsteps_;
				avgy        += globalPositions[s].getY() / nsteps_;
				avgz        += globalPositions[s].getZ() / nsteps_;
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

		auto nsteps_ = edeps.size();
		for (size_t s = 0; s < nsteps_; s++) {
			if (totalEnergyDeposited > 0) {
				avgx += localPositions[s].getX() * edeps[s] / tote;
				avgy += localPositions[s].getY() * edeps[s] / tote;
				avgz += localPositions[s].getZ() * edeps[s] / tote;
			}
			else {
				averageTime += times[s] / nsteps_;
				avgx        += localPositions[s].getX() / nsteps_;
				avgy        += localPositions[s].getY() / nsteps_;
				avgz        += localPositions[s].getZ() / nsteps_;
			}
		}
		avgLocalPosition = G4ThreeVector(avgx, avgy, avgz);
	}
	return avgLocalPosition;
}
