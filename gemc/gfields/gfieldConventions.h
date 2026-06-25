#pragma once

/**
 * @ingroup gfield_module
 * @{
 */

/**
 * \brief Default integration stepper name used when the requested stepper is unsupported.
 *
 * This is a Geant4 stepper class name expressed as a string.
 */
#define GFIELD_DEFAULT_INTEGRATION_STEPPER "G4DormandPrince745"

/**
 * \brief Default minimum step for the chord finder (string with Geant4 units).
 *
 * This value is stored as a string to preserve unit expressions and is parsed later.
 */
#define GFIELD_DEFAULT_MINIMUM_STEP "1.0*mm"

/**
 * \brief Default maximum acceptable field step for Geant4 propagation.
 *
 * A value less than or equal to zero leaves the Geant4 propagator default unchanged.
 */
#define GFIELD_DEFAULT_MAXIMUM_STEP "0*mm"

// Other default parameters are in the form of strings (to preserve unit expressions).

/**
 * \brief Default origin coordinate component for multipole fields (string with Geant4 units).
 */
#define GFIELD_DEFAULT_VERTEX "0*mm"

/**
 * \brief Default multipole roll rotation angle (string with Geant4 units).
 */
#define GFIELD_DEFAULT_ROTANGLE "0*deg"

/**
 * \brief Default verbosity level (string) for field-related configuration.
 */
#define GFIELD_DEFAULT_VERBOSITY "0"

/**
 * \brief Command-line option name used to associate a field with the ROOT world volume.
 *
 * The value is the name of a configured field (see \c gmultipoles / \c gfields). The corresponding
 * \c G4FieldManager is installed on the ROOT world volume and propagated to all daughters, acting as a
 * global field wherever a more specific per-volume field is not set.
 */
#define GLOBAL_FIELD_OPTION "global_field"

/**
 * \brief Command-line option name used to reset (remove) field associations.
 *
 * The value is either a single gvolume name, a whitespace/comma-separated list of gvolume names, or the
 * special value \ref NO_FIELD_ALL. A listed volume that was associated with a field has that association
 * removed; \ref NO_FIELD_ALL removes every per-volume field as well as the \ref GLOBAL_FIELD_OPTION. Fields
 * that end up unused as a result are never loaded (their plugins and maps are skipped).
 */
#define NO_FIELD_OPTION "no_field"

/**
 * \brief Command-line option name used to set the global maximum acceptable field step.
 *
 * The value is parsed as a Geant4 length and passed to \c G4PropagatorInField::SetLargestAcceptableStep()
 * when positive. This controls the largest field propagation step accepted by Geant4 transportation.
 */
#define MAX_FIELD_STEP_OPTION "max_field_step"

/**
 * \brief Special \ref NO_FIELD_OPTION value that resets all fields, including \ref GLOBAL_FIELD_OPTION.
 */
#define NO_FIELD_ALL "all"


// Error codes in the 1200 range (module-local).
#define ERR_WRONG_FIELD_ROTATION 1201
#define ERR_WRONG_FIELD_NOT_FOUND 1202
#define ERR_STEPPER_NOT_FOUND 1203
#define ERR_WRONG_POLE_NUMBER 1204
#define ERR_WRONG_FIELD_SYMMETRY 1205
#define ERR_WRONG_COORDINATE_DEF 1206
#define ERR_MAP_FILE_NOT_FOUND 1207

/** @} */
