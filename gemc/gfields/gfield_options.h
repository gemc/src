#pragma once

// gemc
#include <gemc/goptions/goptions.h>

// gfields
#include "gfield.h"

/**
 * @ingroup gfield_module
 * @{
 */

namespace gfields {

/**
 * \brief Build the list of field definitions from the provided options.
 * \param gopts Shared options container that holds user configuration (e.g. YAML-derived data).
 * \return Vector of field definitions describing which field plugins to load and how to configure them.
 *
 * The returned vector is typically consumed by \ref GMagneto::GMagneto "GMagneto()" during construction.
 */
std::vector<GFieldDefinition> get_GFieldDefinition(const std::shared_ptr<GOptions>& gopts);

/**
 * \brief Define all options used by the GField module and its built-in field factories.
 * \return A GOptions instance containing module option definitions and logger registrations.
 *
 * This function registers:
 * - options for field plugins implemented in this module (currently \c gmultipoles),
 * - and the loggers used by the module (\c gfield, \c gmagneto, and \c plugin).
 */
GOptions defineOptions();

/**
 * \brief Evaluate configured fields from \c fieldAt and \c fieldMapPoints options, if requested.
 * \param gopts Shared options container with field definitions and query options.
 * \return True when at least one query option was present and processed.
 *
 * The query options are intended for command-line inspection workflows:
 * - \c fieldAt accepts one coordinate triplet with units, for example \c "10*cm 0*mm 2*m".
 * - \c fieldMapPoints accepts an ASCII file with one \c x y z coordinate triplet per line.
 */
bool runFieldQueries(const std::shared_ptr<GOptions>& gopts);

} // namespace gfields

/** @} */
