#pragma once

/**
 * @ingroup gfield_module
 * @{
 */

/**
 * @brief Default integration stepper name used when the requested stepper is unsupported.
 *
 * This is a Geant4 stepper class name expressed as a string.
 */
#define GFIELD_DEFAULT_INTEGRATION_STEPPER "G4DormandPrince745"

/**
 * @brief Default minimum step for the chord finder (string with Geant4 units).
 *
 * This value is stored as a string to preserve unit expressions and is parsed later.
 */
#define GFIELD_DEFAULT_MINIMUM_STEP "1.0*mm"

// Other default parameters are in the form of strings (to preserve unit expressions).

/**
 * @brief Default origin coordinate component for multipole fields (string with Geant4 units).
 */
#define GFIELD_DEFAULT_VERTEX "0*mm"

/**
 * @brief Default multipole roll rotation angle (string with Geant4 units).
 */
#define GFIELD_DEFAULT_ROTANGLE "0*deg"

/**
 * @brief Default verbosity level (string) for field-related configuration.
 */
#define GFIELD_DEFAULT_VERBOSITY "0"


// Error codes in the 1200 range (module-local).
#define ERR_WRONG_FIELD_ROTATION 1201
#define ERR_WRONG_FIELD_NOT_FOUND 1202
#define ERR_STEPPER_NOT_FOUND 1203
#define ERR_WRONG_POLE_NUMBER 1204

/** @} */
