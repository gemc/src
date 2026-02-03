#pragma once

#include "goptions.h"

/**
 * @brief Logger channel used by the gfactory module and plugins loaded through it.
 *
 * This string is passed to `GBase` constructors and is typically used to route messages
 * to a dedicated logger configuration for plugins.
 */
constexpr const char* PLUGIN_LOGGER = "plugins";

namespace gfactory {

/**
 * @brief Define the command-line/options set used by the gfactory module.
 *
 * The module exposes its options via a `GOptions` instance so that:
 * - applications can merge these options into their global configuration, and
 * - plugin-related logging/behavior can be configured consistently.
 *
 * @return A `GOptions` instance populated with the options for this module.
 */
GOptions defineOptions();

}
