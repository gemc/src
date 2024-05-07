#ifndef GFIELD_MULTIPOLES_FACTORY_H
#define GFIELD_MULTIPOLES_FACTORY_H 1

// gemc
#include "gfield.h"

class GField_MultipolesFactory : public GField {
public:
    GField_MultipolesFactory() {}

    void create_FieldManager(const GOptions *gopts);

    void GetFieldValue(const G4double pos[4], G4double *bfield) const;

private:

    string field_name;
    int pole_number;
    G4double origin[3];
    G4double rotation_angle;
    int rotaxis; // 0 for X, 1 for Y, 2 for Z
    G4double strength;

};

#endif // GFIELD_MULTIPOLES_FACTORY_H
