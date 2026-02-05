#pragma once

// gemc
#include "goptions.h"

/**
 * @brief Logger name used by the dbselect module.
 *
 * This name is passed to the logging infrastructure (GLogger / GBase) to tag all
 * dbselect-related messages.
 */
constexpr const char* DBSELECT_LOGGER = "dbselect";

/**
 * @ingroup dbselect_module
 * @brief Namespace containing dbselect module functions.
 */
namespace dbselect {

/**
 * @brief Define and return the option set used by the dbselect module.
 *
 * The returned GOptions instance:
 * - is created with the logger name \c DBSELECT_LOGGER
 * - aggregates detector-related options from gdetector::defineOptions()
 *
 * Usage pattern:
 * - Construct a GOptions instance from argc/argv using the returned definition.
 * - Pass the resulting shared options object to \c DBSelectView and detector construction.
 *
 * @return A fully initialized option definition object for dbselect usage.
 */
GOptions defineOptions();

} // namespace dbselect
