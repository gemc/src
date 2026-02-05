#pragma once

/**
 * @file gtree_options.h
 * @ingroup gtree_module
 * @brief Option-set definition entry point for the GTree module.
 *
 * @details
 * The GTree module composes its options through `gtree::defineOptions()`.
 * This module currently aggregates options from the dbselect option set and
 * does not introduce additional GTree-specific switches here.
 */

constexpr const char* GTREE_LOGGER = "gtree";

// gemc
#include "goptions.h"

namespace gtree {

/**
 * @brief Build and return the option set required by the GTree module.
 *
 * @return A fully constructed GOptions instance configured for the `gtree` logger
 *         and augmented with the dbselect option set.
 *
 * @details
 * Typical usage is to pass the returned option set into the application-level
 * GOptions constructor that parses `argc/argv`, then use the resulting shared
 * GOptions instance to construct a GTree widget.
 */
GOptions defineOptions();
}
