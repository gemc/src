#ifndef GDATA_OPTIONS_H
#define GDATA_OPTIONS_H 1

/**
 * \file gdata_options.h
 * \brief Defines the options for the GData module.
 *
 * This file declares a function to build the GOptions object that aggregates
 * options defined in the GData module and its dependent modules.
 */

#include "goptions.h"

namespace gdata {

/**
 * \brief Returns an array of options definitions for the GData module.
 *
 * This function aggregates the options defined by the gdata module and by
 * dependent modules such as gtouchable.
 *
 * \return A GOptions object containing the combined options.
 */
GOptions defineOptions();

} // namespace gdata

#endif
