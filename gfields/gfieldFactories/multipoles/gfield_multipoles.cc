// CLHEP units
#include "CLHEP/Units/PhysicalConstants.h"

// geant4 headers
#include "G4ThreeVector.hh"

// gemc
#include "gfield_multipoles.h"

// c++
#include <iostream>

using namespace std;

//void GField_MultipolesFactory::create_FieldManager([[maybe_unused]] const GOptions *gopts) {
//
//
//}

// for now this implementation follows gemc
// reference of this implementation: https://uspas.fnal.gov/materials/12MSU/magnet_elements.pdf
void GField_MultipolesFactory::GetFieldValue(const G4double pos[4], G4double *bfield) const {
    G4ThreeVector x(pos[0], pos[1], pos[2]);
    G4ThreeVector field_origin(origin[0], origin[1], origin[2]);

    // switch on the rotation axis
    G4ThreeVector x2;
    G4ThreeVector x0_local;
    if (rotaxis == 0) {
        x2 = G4ThreeVector(0 * CLHEP::cm, 0 * CLHEP::cm, 1 * CLHEP::cm).rotateX(rotation_angle) + field_origin;
        x0_local = (x - field_origin).rotateX(-rotation_angle);
    } else if (rotaxis == 1) {
        x2 = G4ThreeVector(0 * CLHEP::cm, 0 * CLHEP::cm, 1 * CLHEP::cm).rotateY(rotation_angle) + field_origin;
        x0_local = (x - field_origin).rotateY(-rotation_angle);
    } else if (rotaxis == 2) {
        x2 = G4ThreeVector(0 * CLHEP::cm, 0 * CLHEP::cm, 1 * CLHEP::cm).rotateZ(rotation_angle) + field_origin;
        x0_local = (x - field_origin).rotateZ(-rotation_angle);
    } else {
        cout << "!!! Error: multipole field has rot axis along X or Y or Z, while there is axis: " << rotaxis << endl;
        exit(EC__WRONG_FIELD_ROTATION);
    }

    G4double r = (x2 - field_origin).cross(field_origin - x).mag() / (x2 - field_origin).mag(); //distance from x0 to line x1-x2
    G4double phi = atan2(x0_local.y(), x0_local.x());

    // calculate the field
    G4ThreeVector B_local;
    if (pole_number == 2) {
        B_local.setX(0);
        B_local.setY(strength);
        B_local.setZ(0);
    } else {
        int a = pole_number / 2 - 1;
        B_local.setX(strength * pow(r/ CLHEP::m, a) * sin(a * phi));
        B_local.setY(strength * pow(r/ CLHEP::m, a) * cos(a * phi));
        B_local.setZ(0);
    }
    G4ThreeVector B_lab=B_local;
    if (rotaxis == 0) {
        B_lab.rotateX(rotation_angle);
    } else if (rotaxis == 1) {
        B_lab.rotateY(rotation_angle);
    } else if (rotaxis == 2) {
        B_lab.rotateZ(rotation_angle);
    }

    bfield[0] = B_lab.x();
    bfield[1] = B_lab.y();
    bfield[2] = B_lab.z();

}
