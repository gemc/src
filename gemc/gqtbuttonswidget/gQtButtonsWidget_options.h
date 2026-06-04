#pragma once

// gemc
#include "goptions.h"

/**
 * \file gQtButtonsWidget_options.h
 * \brief Options definition entry point for the Qt Buttons Widgets module.
 *
 * \details
 * This header declares the option-definition function used by the module to register its command-line
 * and configuration options with GOptions.
 *
 * The module currently defines a logger name constant that can be used when constructing GOptions for
 * this module.
 */

// Logger name used when creating a GOptions instance for this module.
// Note: options are currently minimal; the logger name is retained for consistency.
constexpr const char* GQTBUTTONS_LOGGER = "qtbuttons";

namespace gqtbuttonswidget {
/**
 * \brief Defines the options for the Qt Buttons Widgets module.
 *
 * \details
 * The returned GOptions instance is intended to be merged/consumed by higher-level applications
 * that include this module. The logger name used is \c GQTBUTTONS_LOGGER.
 *
 * \return A configured GOptions instance for this module.
 */
GOptions defineOptions();
}
