/**
 * \file calculated_state.cc
 * \brief Verifies coherent lazy calculation and invalidation of GHit derived values.
 */

#include "ghit.h"
#include "gtouchable.h"
#include "gtouchable_options.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <memory>
#include <numeric>
#include <optional>
#include <string>
#include <vector>

namespace {

bool nearly_equal(double left, double right) {
	const double scale = std::max({1.0, std::abs(left), std::abs(right)});
	return std::abs(left - right) <= 1.0e-12 * scale;
}

bool nearly_equal(const G4ThreeVector& left, const G4ThreeVector& right) {
	return nearly_equal(left.x(), right.x()) && nearly_equal(left.y(), right.y()) &&
	       nearly_equal(left.z(), right.z());
}

} // namespace

int main(int argc, char* argv[]) {
	auto options = std::make_shared<GOptions>(argc, argv, gtouchable::defineOptions());
	auto touchable = std::make_shared<GTouchable>(options, "readout", "sector: 1", std::vector<double>{}, 1.0);
	GHit hit(touchable);

	if (hit.getTotalEnergyDeposited() != 0 || hit.getAverageTime() != 0 ||
	    !nearly_equal(hit.getAvgGlobalPosition(), G4ThreeVector{}) ||
	    !nearly_equal(hit.getAvgLocalPosition(), G4ThreeVector{}) ||
	    hit.getProcessName()) {
		return EXIT_FAILURE;
	}

	hit.randomizeHitForTesting(2);
	if (hit.getMotherInfos().size() != hit.getStepCount() ||
		hit.getMotherInfo().vertex || hit.getMotherInfo().pid) {
		return EXIT_FAILURE;
	}
	const auto energies = hit.getEdeps();
	const auto times = hit.getTimes();
	const auto global_positions = hit.getGlobalPositions();
	const auto local_positions = hit.getLocalPositions();
	const double expected_total = std::accumulate(energies.begin(), energies.end(), 0.0);
	double expected_time = 0;
	G4ThreeVector expected_global;
	G4ThreeVector expected_local;
	for (size_t step = 0; step < energies.size(); ++step) {
		double weight = 1.0 / static_cast<double>(energies.size());
		if (expected_total > 0) { weight = energies[step] / expected_total; }
		expected_time += times[step] * weight;
		expected_global += global_positions[step] * weight;
		expected_local += local_positions[step] * weight;
	}

	if (!nearly_equal(hit.getTotalEnergyDeposited(), expected_total) ||
	    !nearly_equal(hit.getAverageTime(), expected_time) ||
	    !nearly_equal(hit.getAvgGlobalPosition(), expected_global) ||
	    !nearly_equal(hit.getAvgLocalPosition(), expected_local) ||
	    hit.getProcessName() != std::optional<std::string>{"placeholder"}) {
		return EXIT_FAILURE;
	}

	// Appending more step data must invalidate and rebuild the complete cache.
	hit.randomizeHitForTesting(1);
	const auto updated_energies = hit.getEdeps();
	const double updated_total = std::accumulate(updated_energies.begin(), updated_energies.end(), 0.0);
	return nearly_equal(hit.getTotalEnergyDeposited(), updated_total) ? EXIT_SUCCESS : EXIT_FAILURE;
}
