#pragma once

/**
 * \file gdynamicdigitization_options.h
 * \brief Defines the options for the GDynamicDigitization module.
 *
 * This file declares a function to build the GOptions object that aggregates
 * options defined in the GData module and its dependent modules.
 */

#include "goptions.h"

constexpr const char* GDIGITIZATION_LOGGER = "gdigitization";

namespace gdynamicdigitization {

/**
 * \brief Returns an array of options definitions for the GDynamicDigitization module.
 *
 * This function aggregates the options defined by the gdata module and by
 * dependent modules such as gtranslationTable.
 *
 * \return A GOptions object containing the combined options.
 */
GOptions defineOptions();

} // namespace gdynamicdigitization

