#pragma once

// gemc
#include "gfield.h"

/**
 * @brief Factory class for creating and managing multipole magnetic fields.
 *
 * This class represents a specific implementation of a magnetic field characterized by multipoles.
 * It inherits from the GField base class.
 */
class GField_MultipolesFactory : public GField {

public:
    /**
     * @brief Default constructor for the GField_MultipolesFactory class.
     */
    //GField_MultipolesFactory() = default;
	explicit GField_MultipolesFactory(const std::shared_ptr<GOptions>& gopt) : GField(gopt) {}

    /**
     * @brief Calculates the magnetic field at a given position.
     * @param pos Array containing the position (x, y, z, time).
     * @param bfield Output array to store the magnetic field vector (Bx, By, Bz).
     */
    void GetFieldValue(const double pos[3], G4double *bfield) const override;

    void load_field_definitions(GFieldDefinition gfd) override;

private:

    int pole_number; ///< Number of poles in the multipole field.
    G4double origin[3]; ///< Origin of the multipole field (x, y, z).
    G4double rotation_angle; ///< Rotation angle of the field in degrees.
    int rotaxis; ///< Axis of rotation: 0 for X, 1 for Y, 2 for Z.
    G4double strength; ///< Strength of the multipole field.
	bool longitudinal; ///< Longitudinal or transverse field.

};


