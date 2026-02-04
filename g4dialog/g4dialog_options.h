#pragma once

// gemc
#include "goptions.h"

/**
 * \brief Logger name used by the G4Dialog module.
 *
 * \details
 * This string is passed to the logging infrastructure (via GBase/Glogger integration)
 * to categorize messages produced by this module.
 */
constexpr const char* G4DIALOG_LOGGER = "g4dialog";

namespace g4dialog {
/**
 * \brief Define the command-line and configuration options for the G4Dialog module.
 *
 * \details
 * The returned GOptions object is typically merged/constructed by the surrounding framework
 * (for example, a top-level application that instantiates GOptions and then uses the module's
 * option schema).
 *
 * \return A GOptions instance preconfigured with this module's option definitions.
 */
GOptions defineOptions();
} // namespace g4dialog
