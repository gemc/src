#pragma once

#include "goptions.h"

/**
 * \file gtranslationTable_options.h
 * \brief Option definitions for the Translation Table module.
 *
 * This module defines its own option set so it can be configured consistently using the
 * project-wide options mechanism.
 *
 * The logger name associated with this module is \c "gtranslationtable".
 */

constexpr const char* TRANSLATIONTABLE_LOGGER = "gtranslationtable";

namespace gtranslationTable {
/**
 * \brief Defines the module option set for gtranslationTable.
 *
 * The returned GOptions instance is typically passed to the project option parser/manager,
 * and used to configure logging and other module-level behavior.
 *
 * \return A GOptions instance containing the option definitions for this module.
 */
GOptions defineOptions();
}
