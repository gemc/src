/**
 * \file modifier_transform.cc
 * \brief Verifies that optional geometry shift and tilt modifiers affect Geant4 transforms.
 */

#include "g4objectsFactory.h"

#include <CLHEP/Units/SystemOfUnits.h>

#include <cmath>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

namespace {

class TransformProbe : public G4ObjectsFactory
{
public:
	using G4ObjectsFactory::getPosition;
	using G4ObjectsFactory::getRotation;
};

bool close_to(double actual, double expected) {
	return std::abs(actual - expected) < 1.0e-9;
}

} // namespace

int main() {
	const std::shared_ptr<GLogger> logger;
	GVolume volume(logger, "test", {
		"modified", "G4Box", "1*cm, 1*cm, 1*cm", "G4_AIR", "root",
		"1*cm, 2*cm, 3*cm", "0*deg, 0*deg, 0*deg", "active", "NULL",
		"1", "1", "778899", "1", "NULL", "NULL", "NULL", "NULL", "NULL",
		"1", "modifier transform test", "default", "1"
	});
	volume.applyShift("4*cm, 5*cm, 6*cm");
	volume.applyTilt("0*deg, 0*deg, 90*deg");

	const auto position = TransformProbe::getPosition(&volume);
	if (!close_to(position.x(), 5.0 * CLHEP::cm) ||
	    !close_to(position.y(), 7.0 * CLHEP::cm) ||
	    !close_to(position.z(), 9.0 * CLHEP::cm)) {
		return EXIT_FAILURE;
	}

	std::unique_ptr<G4RotationMatrix> rotation(TransformProbe::getRotation(&volume));
	const auto rotated = *rotation * G4ThreeVector(1.0, 0.0, 0.0);
	if (!close_to(rotated.x(), 0.0) || !close_to(rotated.y(), 1.0) || !close_to(rotated.z(), 0.0)) {
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
