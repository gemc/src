#pragma once

// gemc
#include "gfield.h"

/**
 * @defgroup gfield_multipoles_factory GField multipoles factory
 * @ingroup gfield_module
 *
 * @brief Multipole magnetic-field plugin implementation.
 *
 * This group contains the implementation of the multipole magnetic field factory used by the
 * GField framework. The plugin is dynamically loaded by name via GFieldDefinition::gfieldPluginName().
 */

/**
 * @brief Factory class implementing an ideal multipole magnetic field.
 * @ingroup gfield_multipoles_factory
 *
 * This class is a concrete GField implementation that evaluates an idealized multipole field
 * using common accelerator-style conventions:
 * - The transverse field scales like `r^(n-1)` where `n = pole_number/2`.
 * - The angular dependence is expressed via `cos((n-1)*phi)` and `sin((n-1)*phi)`.
 *
 * The field is evaluated in a local magnet frame centered at \c origin and then rotated back
 * into the lab frame by \c rotation_angle around \c rotaxis.
 *
 * Two operating modes exist:
 * - **Transverse multipole mode** (\c longitudinal == false): produces a 2D multipole field in the plane
 *   perpendicular to \c rotaxis.
 * - **Longitudinal mode** (\c longitudinal == true): produces a uniform axial field aligned with \c rotaxis
 *   (solenoid-like); this mode is not a multipole expansion, but is provided for convenience/configuration symmetry.
 *
 * Units:
 * - \c origin and the evaluation point \c pos are expected in Geant4 length units (e.g. \c mm).
 * - \c strength is expected in Tesla.
 * - \c rotation_angle is expected in Geant4 angle units (e.g. \c deg).
 */
class GField_MultipolesFactory : public GField {

public:
	/**
	 * @brief Construct the multipole field factory using the module options.
	 * @param gopt Shared options object used by the base class (logging, configuration).
	 *
	 * The options are owned externally and are expected to outlive this object.
	 */
	explicit GField_MultipolesFactory(const std::shared_ptr<GOptions>& gopt) : GField(gopt) {}

	/**
	 * @brief Evaluate the magnetic field B at a given position.
	 * @param pos Position in the lab frame as \c {x,y,z}.
	 * @param bfield Output array filled with \c {Bx,By,Bz} in Tesla.
	 *
	 * This method is the Geant4 entry point used by the field propagator. The implementation:
	 * 1. Translates the query point into magnet-centered coordinates using \c origin.
	 * 2. Applies an “unroll” rotation by \c -rotation_angle around \c rotaxis.
	 * 3. Computes the field in the transverse plane (or axial mode if enabled).
	 * 4. Rotates the field back to the lab frame and writes it to \c bfield.
	 */
	void GetFieldValue(const double pos[3], G4double* bfield) const override;

	/**
	 * @brief Load and cache field-definition parameters for fast field evaluation.
	 * @param gfd Field definition populated from options.
	 *
	 * The input definition is copied into the base class storage and the frequently used parameters
	 * are parsed and cached into member variables (e.g. pole number, origin, rotation).
	 */
	void load_field_definitions(GFieldDefinition gfd) override;

private:
	/// Number of poles of the ideal multipole: 2=dipole, 4=quadrupole, 6=sextupole, ...
	int pole_number = 0;

	/// Magnet origin (lab frame) used to translate query points into a local magnet-centered frame.
	G4double origin[3] = {0.0, 0.0, 0.0};

	/// Roll rotation angle applied about \c rotaxis (Geant4 angle units, typically \c deg).
	G4double rotation_angle = 0.0;

	/**
	 * @brief Axis index defining the magnet longitudinal axis and rotation axis.
	 *
	 * Encoding:
	 * - 0 : X axis
	 * - 1 : Y axis
	 * - 2 : Z axis
	 */
	int rotaxis = 2;

	/**
	 * @brief Multipole strength in Tesla at the reference radius (currently fixed to 1 m in the implementation).
	 *
	 * In transverse multipole mode this is the field magnitude at `r=r0` for the effective order.
	 * In longitudinal mode it is the uniform axial field magnitude.
	 */
	G4double strength = 0.0;

	/// If true, return a uniform axial field aligned with \c rotaxis.
	bool longitudinal = false;
};
