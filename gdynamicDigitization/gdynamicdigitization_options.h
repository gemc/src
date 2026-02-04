#pragma once

/**
 * \file gdynamicdigitization_options.h
 * \brief Options for the GDynamicDigitization module.
 *
 * This header declares the option aggregation entry point used by programs and plugins
 * that rely on gdynamic digitization infrastructure.
 */

#include "goptions.h"

/// Logger name used by this module.
constexpr const char* GDIGITIZATION_LOGGER = "gdigitization";

namespace gdynamicdigitization {

/**
 * \brief Builds and returns the option set for gdynamic digitization.
 *
 * The returned GOptions includes:
 * - module-specific switches (e.g. \c recordZeroEdep)
 * - options aggregated from dependent subsystems needed by typical workflows
 *
 * \return A populated GOptions object.
 */
GOptions defineOptions();

} // namespace gdynamicdigitization
