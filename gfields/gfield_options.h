#pragma once

// gemc
#include "goptions.h"

// gfields
#include "gfield.h"

/**
 * @ingroup gfield_module
 * @{
 */

namespace gfields {

/**
 * @brief Build the list of field definitions from the provided options.
 * @param gopts Shared options container that holds user configuration (e.g. YAML-derived data).
 * @return Vector of field definitions describing which field plugins to load and how to configure them.
 *
 * The returned vector is typically consumed by \ref GMagneto::GMagneto "GMagneto()" during construction.
 */
std::vector<GFieldDefinition> get_GFieldDefinition(const std::shared_ptr<GOptions>& gopts);

/**
 * @brief Define all options used by the GField module and its built-in field factories.
 * @return A GOptions instance containing module option definitions and logger registrations.
 *
 * This function registers:
 * - options for field plugins implemented in this module (currently \c gmultipoles),
 * - and the loggers used by the module (\c gfield, \c gmagneto, and \c plugin).
 */
GOptions defineOptions();

} // namespace gfields

/** @} */
