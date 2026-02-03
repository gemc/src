#pragma once

#include "goptions.h"

/**
 * @file gtouchable_options.h
 * @brief Options definition entry point for the gtouchable module.
 */

/**
 * @brief Logger name used by the gtouchable module.
 *
 * This constant is used to namespace log output associated with \c GTouchable and related helpers.
 */
constexpr const char* TOUCHABLE_LOGGER = "gtouchable";

namespace gtouchable {
/**
 * @brief Defines the module options supported by gtouchable.
 *
 * The returned options set is used to configure logging and any module-level behavior
 * that may be introduced in the future.
 *
 * @return A \c GOptions object describing the supported options for this module.
 */
GOptions defineOptions();
} // namespace gtouchable
