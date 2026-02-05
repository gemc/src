#pragma once

// glibrary
#include "goptions.h"

constexpr const char* GPHYSICS_LOGGER = "gphysics";

namespace gphysics {

/**
 * @ingroup gphysics_module
 *
 * @brief Defines the user-facing options for the gphysics module.
 *
 * The returned GOptions instance defines:
 * - \c phys_list : selects the reference physics list string (with optional extensions).
 * - \c showPhysics : prints available physics lists and constructors and exits.
 *
 * The help text includes the Geant4 version tag and a curated list of common reference lists and
 * constructors available in the linked Geant4 build.
 *
 * @return A fully constructed GOptions object containing gphysics option definitions.
 */
GOptions defineOptions();

}
