#pragma once

namespace gphysics {

/**
 * @ingroup gphysics_module
 *
 * \brief Default value for the \c phys_list option.
 *
 * This string is used by gphysics::defineOptions() when the user does not explicitly specify
 * a reference physics list.
 */
inline constexpr char DEFAULTPHYSICSLIST[] = "FTFP_BERT";

/**
 * @ingroup gphysics_module
 *
 * \brief Error code used when the requested physics list cannot be loaded.
 *
 * This code is reported by GPhysics when the Geant4 factory fails to produce a physics list
 * from the user-provided \c phys_list string.
 */
inline constexpr int ERR_PHYSLISTERROR = 401;

} // namespace gphysics
