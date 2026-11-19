#include <gemc/gfields/gfield_options.h>
#include <gemc/gfields/gmagneto.h>

#include "G4SystemOfUnits.hh"

#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

namespace {

using Vector = std::array<double, 3>;

struct Sample {
	const char* name;
	Vector position;
	Vector expected;
};

Vector evaluate(const std::shared_ptr<GField>& field, const Vector& position) {
	Vector value{};
	field->GetFieldValue(position.data(), value.data());
	return value;
}

bool near(double value, double expected, double tolerance, const std::string& label) {
	if (std::isfinite(value) && std::abs(value - expected) <= tolerance) { return true; }
	std::cerr << label << ": expected " << expected << ", got " << value << '\n';
	return false;
}

bool check_focusing(const std::string& name, const std::shared_ptr<GField>& field, bool x_focusing) {
	// For a positive charge moving along +Z, F/(q*v) = (-By, Bx, 0).
	// With G=1 T/m, the normal quadrupole restores X and defocuses Y; a 90-degree roll swaps them.
	bool passed = true;
	for (int axis = 0; axis < 2; ++axis) {
		const bool focusing = (axis == 0) == x_focusing;
		for (const double offset : {-10*mm, 10*mm}) {
			Vector position{};
			position[axis] = offset;
			const auto value = evaluate(field, position);
			const Vector force_per_charge_speed{-value[1], value[0], 0};
			Vector expected{};
			expected[axis] = (focusing ? -1.0 : 1.0) * offset / m * tesla;
			const std::string label = name + (axis == 0 ? " X " : " Y ") +
			                          (focusing ? "focusing" : "defocusing") +
			                          (offset < 0 ? " at negative offset" : " at positive offset");
			for (int component = 0; component < 3; ++component) {
				passed &= near(force_per_charge_speed[component], expected[component], 1e-10*tesla,
				               label + " force/(q*v) component " + std::to_string(component));
			}
		}
	}
	return passed;
}

bool check_maxwell(const std::string& name, const std::shared_ptr<GField>& field) {
	const Vector position{0.1*m, 0.2*m, 0.3*m};
	const double h = 0.1*mm;
	double derivative[3][3]{};
	for (int axis = 0; axis < 3; ++axis) {
		auto plus = position;
		auto minus = position;
		plus[axis] += h;
		minus[axis] -= h;
		const auto bp = evaluate(field, plus);
		const auto bm = evaluate(field, minus);
		for (int component = 0; component < 3; ++component) {
			derivative[component][axis] = (bp[component] - bm[component]) / (2*h);
		}
	}

	const double tolerance = 1e-9*tesla/m;
	bool passed = near(derivative[0][0] + derivative[1][1] + derivative[2][2],
	                   0.0, tolerance, name + " divergence");
	passed &= near(derivative[2][1] - derivative[1][2], 0.0, tolerance, name + " curl X");
	passed &= near(derivative[0][2] - derivative[2][0], 0.0, tolerance, name + " curl Y");
	passed &= near(derivative[1][0] - derivative[0][1], 0.0, tolerance, name + " curl Z");
	return passed;
}

} // namespace

int main(int argc, char* argv[]) {
	auto options = std::make_shared<GOptions>(argc, argv, gfields::defineOptions());
	GMagneto magneto(options);

	// Independent reference vectors for G=1 T/m, and the normal sextupole at r0=1 m.
	const Sample samples[] = {
		{"quad_z", {10*mm, 20*mm, 0}, {0.02*tesla, 0.01*tesla, 0}},
		{"quad_z", {-20*mm, 10*mm, 0}, {0.01*tesla, -0.02*tesla, 0}},
		{"quad_z", {0, 0, 0}, {0, 0, 0}},
		{"quad_x", {0, 10*mm, 20*mm}, {0, 0.02*tesla, 0.01*tesla}},
		{"quad_y", {20*mm, 0, 10*mm}, {0.01*tesla, 0, 0.02*tesla}},
		{"quad_shifted", {40*mm, -20*mm, 50*mm}, {0.02*tesla, 0.01*tesla, 0}},
		{"quad_rolled", {10*mm, 20*mm, 0}, {-0.01*tesla, 0.02*tesla, 0}},
		{"sextupole", {0.1*m, 0.2*m, 0}, {0.04*tesla, -0.03*tesla, 0}},
		{"sextupole", {-0.1*m, 0.2*m, 0}, {-0.04*tesla, -0.03*tesla, 0}},
		{"dipole", {10*mm, 20*mm, 30*mm}, {1*tesla, 0, 0}},
		{"dipole_rolled", {10*mm, 20*mm, 30*mm},
		 {0.7071067811865475*tesla, 0.7071067811865475*tesla, 0}},
		{"longitudinal", {10*mm, 20*mm, 30*mm}, {0, 1*tesla, 0}},
	};

	bool passed = true;
	for (const auto& sample : samples) {
		const auto value = evaluate(magneto.getField(sample.name), sample.position);
		for (int component = 0; component < 3; ++component) {
			passed &= near(value[component], sample.expected[component], 1e-10*tesla,
			               std::string(sample.name) + " component " + std::to_string(component));
		}
	}
	for (const auto& name : magneto.getFieldNames()) {
		passed &= check_maxwell(name, magneto.getField(name));
	}
	passed &= check_focusing("quad_z", magneto.getField("quad_z"), true);
	passed &= check_focusing("quad_rolled_90", magneto.getField("quad_rolled_90"), false);
	if (!passed) { return EXIT_FAILURE; }
	std::cout << "Multipole reference fields, focusing/defocusing, roll, divergence/curl, "
	             "and dipole preservation passed.\n";
	return EXIT_SUCCESS;
}
