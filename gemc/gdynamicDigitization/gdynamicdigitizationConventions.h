#pragma once

/**
 * \file gdynamicdigitizationConventions.h
 * \brief Exit/error codes used by gdynamic digitization.
 *
 * These constants are used as exit codes (or error identifiers) for common failure
 * scenarios in dynamic digitization workflows.
 */

/// Exit code when a detector is not found.
constexpr int ERR_LOADTTFAIL = 1601;

/// Exit code when a variable (constant/calibration) is not found.
constexpr int ERR_LOADCONSTANTFAIL = 1602;

/// Exit code when a payload is of the wrong size / unexpected format.
constexpr int ERR_DEFINESPECFAIL = 1603;
