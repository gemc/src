/**
* @file gactionConventions.h
 * @brief Defines error codes used by the GEMC actions module.
 *
 * This header centralizes stable error identifiers used by the action-related
 * classes when reporting failures through the logging system.
 *
 * @defgroup gactions_module_errors GEMC actions module error codes
 * @brief Error/status codes used by the GEMC actions module.
 *
 * These numeric codes make it easier to identify recurring failure modes in logs,
 * tests, and operational diagnostics.
 *
 * @ingroup gactions_module
 */

/**
 * @ingroup gactions_module_errors
 * @name Error codes
 * @brief Module-level error/status codes emitted by action-related classes.
 *
 * The values are in the 1200 range and are intended to remain stable so log-based
 * diagnostics can reliably identify the same category of problem across builds.
 */
///@{
#define ERR_GRUNACTION_NOT_EXISTING 1201
#define ERR_GDIGIMAP_NOT_EXISTING 1202
#define ERR_STREAMERMAP_NOT_EXISTING 1203
///@}