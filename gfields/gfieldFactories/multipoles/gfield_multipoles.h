#ifndef GFIELD_MULTIPOLES_FACTORY_H
#define GFIELD_MULTIPOLES_FACTORY_H 1

// gemc
#include "gfield.h"

class GField_MultipolesFactory : public GField {

public:

    GField_MultipolesFactory() {}

    void GetFieldValue(const G4double pos[4], G4double *bfield) const;

    void set_parameters(map<string, string> parameters) {
        set_basic_parameters(parameters);
        pole_number = stoi(parameters["pole_number"]);
        origin[0] = stod(parameters["vx"]);
        origin[1] = stod(parameters["vy"]);
        origin[2] = stod(parameters["vz"]);
        rotation_angle = stod(parameters["rotation_angle"]);
        if (parameters["rotaxis"] == "X") {
            rotaxis = 0;
        } else if (parameters["rotaxis"] == "Y") {
            rotaxis = 1;
        } else if (parameters["rotaxis"] == "Z") {
            rotaxis = 2;
        } else {
            std::cout << "!!! Error: multipole field has rot axis along X or Y or Z, while there is axis: " << rotaxis << std::endl;
            exit(EC__WRONG_FIELD_ROTATION);
        }
        strength = stod(parameters["strength"]);
    }


private:

    string field_name;
    int pole_number;
    G4double origin[3];
    G4double rotation_angle;
    int rotaxis; // 0 for X, 1 for Y, 2 for Z
    G4double strength;

};

#endif // GFIELD_MULTIPOLES_FACTORY_H
