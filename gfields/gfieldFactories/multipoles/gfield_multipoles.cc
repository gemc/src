// CLHEP units
#include "CLHEP/Units/PhysicalConstants.h"

// geant4 headers
#include "G4ThreeVector.hh"

// gfields
#include "gfield_multipoles.h"

// gemv
#include "gutilities.h"

// c++
#include <iostream>

using namespace std;

// tells the DLL how to create a GFieldFactory
extern "C" GField *GFieldFactory(void) {
    return static_cast<GField *>(new GField_MultipolesFactory);
}


// for now this implementation follows gemc
// reference of this implementation: https://uspas.fnal.gov/materials/12MSU/magnet_elements.pdf
void GField_MultipolesFactory::GetFieldValue(const G4double pos[4], G4double *bfield) const {

    G4ThreeVector x0(pos[0], pos[1], pos[2]);
    G4ThreeVector x1(origin[0], origin[1], origin[2]);
    G4ThreeVector x2;
    G4ThreeVector x0_local;
    if (rotaxis == 0) {
        x2 = G4ThreeVector(0 * CLHEP::cm, 0 * CLHEP::cm, 1 * CLHEP::cm).rotateX(rotation_angle) + x1;
        x0_local = (x0 - x1).rotateX(-rotation_angle);
    } else if (rotaxis == 1) {
        x2 = G4ThreeVector(0 * CLHEP::cm, 0 * CLHEP::cm, 1 * CLHEP::cm).rotateY(rotation_angle) + x1;
        x0_local = (x0 - x1).rotateY(-rotation_angle);
    } else if (rotaxis == 2) {
        x2 = G4ThreeVector(0 * CLHEP::cm, 0 * CLHEP::cm, 1 * CLHEP::cm).rotateZ(rotation_angle) + x1;
        x0_local = (x0 - x1).rotateZ(-rotation_angle);
    }

    G4double r = (x2 - x1).cross(x1 - x0).mag() / (x2 - x1).mag(); //distance from x0 to line x1-x2
    G4double phi = atan2(x0_local.y(), x0_local.x());

    G4ThreeVector B_local;
    if (pole_number == 2) {
        B_local.setX(0);
        B_local.setY(strength);
        B_local.setZ(0);
    } else if (pole_number > 0) {
        int a = pole_number / 2 - 1;
        B_local.setX(strength * pow(r / CLHEP::m, a) * sin(a * phi));
        B_local.setY(strength * pow(r / CLHEP::m, a) * cos(a * phi));
        B_local.setZ(0);
    } else {
        cout << GFIELDLOGHEADER << "GField_MultipolesFactory::GetFieldValue: Pole number " + to_string(pole_number) + " not supported. Exiting." << endl;
        exit(EC__WRONG_POLE_NUMBER);
    }

    G4ThreeVector B_lab = B_local;
    if (rotaxis == 0) { B_lab.rotateX(rotation_angle); }
    else if (rotaxis == 1) { B_lab.rotateY(rotation_angle); }
    else if (rotaxis == 2) { B_lab.rotateZ(rotation_angle); }

    bfield[0] = 10000 * B_lab.x();
    bfield[1] = 10000 * B_lab.y();
    bfield[2] = 10000 * B_lab.z();

//    cout << " Pole number: " << pole_number << endl;
//    cout << " Strength: " << strength << endl;
//    cout << " Origin: " << origin[0] << " " << origin[1] << " " << origin[2] << endl;
//    cout << " Rotation angle: " << rotation_angle << endl;
//    cout << " Rotation axis: " << rotaxis << endl;
//    cout << "B_lab.x() = " << B_lab.x() << endl;
//    cout << "B_lab.y() = " << B_lab.y() << endl;
//    cout << "B_lab.z() = " << B_lab.z() << endl;
//    cout << " B Field x = " << bfield[0] << endl;
//    cout << " B Field y = " << bfield[1] << endl;
//    cout << " B Field z = " << bfield[2] << endl;

}

void GField_MultipolesFactory::load_field_definitions(GFieldDefinition gfd) {
    gfield_definitions = gfd;

    pole_number =  get_field_parameter_int("pole_number");
    origin[0] = get_field_parameter_double("vx");
    origin[1] = get_field_parameter_double("vy");
    origin[2] = get_field_parameter_double("vz");
    rotation_angle = get_field_parameter_double("rotation_angle");
    if( gfield_definitions.field_parameters["rotaxis"] == "X") {
        rotaxis = 0;
    } else if( gfield_definitions.field_parameters["rotaxis"] == "Y") {
        rotaxis = 1;
    } else if( gfield_definitions.field_parameters["rotaxis"] == "Z") {
        rotaxis = 2;
    } else {
        cout << GFIELDLOGHEADER << "GField_MultipolesFactory::load_field_definitions: Rotation axis " + gfield_definitions.field_parameters["rotaxis"] + " not supported. Exiting." << endl;
        exit(EC__WRONG_FIELD_ROTATION);
    }
    strength = get_field_parameter_double("strength");
}
