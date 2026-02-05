#pragma once
/**
 * @file   g4system_options.h
 * @ingroup g4system
 * @brief  Option definitions for the g4system module (geometry/material construction layer).
 */

// gemc
#include "goptions.h"

/**
 * @brief Logger name used by the module-level builder (e.g. \c G4World).
 *
 * @details
 * This logger is typically used for high-level phases such as:
 * - registering factories
 * - building materials and defaults
 * - resolving volume dependency order
 */
constexpr const char* G4SYSTEM_LOGGER = "g4system";

/**
 * @brief Logger name used by object factories (e.g. \c G4ObjectsFactory derived classes).
 *
 * @details
 * This logger is typically more verbose and focuses on per-volume operations:
 * - solid/logical/physical construction
 * - material fallback behavior
 * - dependency checks and placement diagnostics
 */
constexpr const char* G4SFACTORY_LOGGER = "g4sfactory";

namespace g4system {
/**
 * @brief Define command-line/configuration options for the g4system module.
 *
 * @details
 * The returned option set includes:
 * - module loggers and their verbosity control
 * - geometry/material related diagnostic switches
 * - overlap checking configuration
 * - optional backup material used when a requested material does not exist
 *
 * This is typically passed to the global GOptions constructor so that options are registered
 * before any geometry building begins.
 *
 * @return A fully-populated option definition set for this module.
 */
GOptions defineOptions();
}
