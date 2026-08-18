// ghit
#include "ghit.h"

// c++
#include <utility>

// See header for API docs.

const GHit::CalculatedState& GHit::getCalculatedState() const {
	if (calculatedState) return *calculatedState;

	CalculatedState state;
	for (const double edep : edeps) { state.totalEnergyDeposited += edep; }
	if (!processNames.empty()) { state.processName = processNames.front(); }

	const size_t step_count = edeps.size();
	if (step_count > 0) {
		const bool energy_weighted = state.totalEnergyDeposited > 0;
		for (size_t step = 0; step < step_count; ++step) {
			double weight = 1.0 / static_cast<double>(step_count);
			if (energy_weighted) { weight = edeps[step] / state.totalEnergyDeposited; }
			state.averageTime += times[step] * weight;
			state.averageGlobalPosition += globalPositions[step] * weight;
			state.averageLocalPosition += localPositions[step] * weight;
		}
	}

	calculatedState = std::move(state);
	return *calculatedState;
}

void GHit::invalidateCalculatedState() {
	calculatedState.reset();
}

double GHit::getTotalEnergyDeposited() const {
	return getCalculatedState().totalEnergyDeposited;
}

double GHit::getAverageTime() const {
	return getCalculatedState().averageTime;
}

G4ThreeVector GHit::getAvgGlobalPosition() const {
	return getCalculatedState().averageGlobalPosition;
}

G4ThreeVector GHit::getAvgLocalPosition() const {
	return getCalculatedState().averageLocalPosition;
}

const std::optional<std::string>& GHit::getProcessName() const {
	return getCalculatedState().processName;
}
