#pragma once

namespace gemc {

/**
 * \file gemcConventions.h
 * \brief Conventional constants used by GEMC utility helpers.
 *
 * This header defines small numeric/string constants that are used across
 * utility routines (for example, error codes and default configuration values).
 */

/**
 * @defgroup gemc_utilities_conventions GEMC Utilities Conventions
 * \brief Constants and conventional values used by GEMC utility helpers.
 *
 * The constants in this group are used by utility helpers to provide
 * consistent defaults and error reporting identifiers.
 * @{
 */

/**
 * \brief Error code used when the configured random engine name is not recognized.
 *
 * This error code is used by the random engine selection logic when the `randomEngine`
 * option does not match any supported engine name.
 */
inline constexpr int EC__RANDOMENGINENOTFOUND = 2003;

/**
 * \brief Default random engine name used when no explicit selection is provided.
 *
 * The value is expected to match one of the CLHEP engine names handled by the
 * random engine selection logic.
 */
inline constexpr char DEFAULT_RANDOM_ENGINE[] = "RanluxEngine";

/** @} */ // end of group gemc_utilities_conventions

} // namespace gemc
