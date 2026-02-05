#pragma once

// gemc
#include "goptions.h"

/**
 * @file gdetector_options.h
 * @brief Declares the gdetector module option aggregation entry point.
 *
 * @ingroup gdetector_module
 */

/**
 * @brief Logger name used by the gdetector module.
 *
 * The module uses this string to tag log output and to associate the correct
 * verbosity/diagnostic behavior through the logging infrastructure.
 */
constexpr const char* GDETECTOR_LOGGER = "gdetector";

namespace gdetector {

/**
 * @brief Defines and returns the aggregated options for the gdetector module.
 *
 * This function collects options from the gdetector module and from the
 * submodules it depends on. The returned object is typically passed to a
 * GOptions constructor so that command-line parsing and defaults are applied
 * consistently.
 *
 * Aggregated option sets currently include:
 * - gsystem options (system/volume definitions and related configuration)
 * - g4system options (Geant4-world construction helpers)
 * - gdynamicdigitization options (digitization plugin loading/config)
 * - gsensitivedetector options (SD configuration and conventions)
 *
 * @return A populated GOptions object that contains all options used by this module.
 */
GOptions defineOptions();

}
