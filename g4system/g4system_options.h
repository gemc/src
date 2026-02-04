#pragma once
/**
 * @file   g4system_options.h
 * @ingroup Geometry
 * @brief  Option definitions for the g4system module (geometry/material construction layer).
 */

// gemc
#include "goptions.h"

/** Logger name used by the module-level builder (e.g. \c G4World). */
constexpr const char* G4SYSTEM_LOGGER = "g4system";

/** Logger name used by object factories (e.g. \c G4ObjectsFactory derived classes). */
constexpr const char* G4SFACTORY_LOGGER = "g4sfactory";

namespace g4system {
/**
 * @brief Define command-line/configuration options for the g4system module.
 *
 * @details
 * The returned GOptions instance contains:
 * - module loggers
 * - geometry/material related switches
 * - overlap checking configuration
 * - optional backup material used when a requested material does not exist
 *
 * This is typically passed to the global GOptions constructor so that options are registered
 * before any geometry building begins.
 *
 * @return A fully-populated GOptions definition set for this module.
 */
GOptions defineOptions();
}
