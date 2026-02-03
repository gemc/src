#pragma once

/**
 * \file gtranslationTableConventions.h
 * \brief Conventions (primarily exit/error codes) for the Translation Table module.
 *
 * This header centralizes numeric identifiers used for logging and error reporting
 * in the Translation Table module so they remain stable and consistent across builds.
 *
 * \note These values are intended to be used with the module logger (see the module
 *       mainpage in gtranslationTableDoxy.h).
 */

// exit codes: 1100s

/**
 * \def EC__GIDENTITYNOTFOUNDINTT
 * \brief Error code used when a requested identity cannot be resolved in the translation table context.
 *
 * Typical use: emitted when an identity vector (or derived key) is invalid or cannot be found.
 */
#define EC__GIDENTITYNOTFOUNDINTT 1101

/**
 * \def EC__TTNOTFOUNDINTT
 * \brief Error code used when a translation table entry is not found for a given key.
 *
 * Typical use: emitted by API calls that retrieve electronics for a given identity.
 */
#define EC__TTNOTFOUNDINTT        1102
