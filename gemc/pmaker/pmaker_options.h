#pragma once

// gemc
#include <gemc/goptions/goptions.h>

constexpr const char* PMAKER_LOGGER = "pmaker";

namespace pmaker {

/**
 * \brief Define and return the option set used by the pmaker module.
 *
 * Registering a GOptions with PMAKER_LOGGER makes the verbosity and debug
 * switches available on the command line for this module.
 *
 * \return A GOptions instance initialised for the pmaker logger.
 */
GOptions defineOptions();

} // namespace pmaker
