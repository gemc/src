#pragma once

// glibrary
#include "goptions.h"

/**
 * \file eventDispenser_options.h
 * \brief Public declaration of the Event Dispenser module command-line / configuration options.
 *
 * This header declares the module entry point used to define the set of \c GOptions handled
 * by the Event Dispenser module (and the modules it depends on).
 */

/**
 * \brief Logger name used by this module when creating a \c GLogger through the base infrastructure.
 *
 * This string is passed to the logging backend so messages from this module can be filtered
 * and formatted consistently.
 */
constexpr const char* EVENTDISPENSER_LOGGER = "eventdispenser";

/**
 * \namespace eventDispenser
 * \brief Namespace containing the Event Dispenser module option definitions.
 */
namespace eventDispenser {
/**
 * \brief Builds and returns the complete set of options supported by the Event Dispenser module.
 *
 * The returned \c GOptions object:
 * - Defines Event Dispenser specific options (e.g. number of events, run number, run-weight file).
 * - Appends options required by dependent modules (e.g. dynamic digitization).
 *
 * \details
 * This function is intended to be used during application startup to initialize \c GOptions,
 * typically by passing the returned object into the \c GOptions constructor that parses
 * \c argc/\c argv.
 *
 * \return A \c GOptions instance containing all option definitions for this module.
 */
GOptions defineOptions();
} // namespace eventDispenser
