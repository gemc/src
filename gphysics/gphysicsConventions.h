#pragma once

/**
 * @ingroup gphysics_module
 *
 * @brief Default value for the \c phys_list option.
 *
 * This string is used by gphysics::defineOptions() when the user does not explicitly specify
 * a reference physics list.
 */
#define DEFAULTPHYSICSLIST "FTFP_BERT"

/**
 * @ingroup gphysics_module
 *
 * @brief Error code used when the requested physics list cannot be loaded.
 *
 * This code is reported by GPhysics when the Geant4 factory fails to produce a physics list
 * from the user-provided \c phys_list string.
 */
#define ERR_PHYSLISTERROR 401
