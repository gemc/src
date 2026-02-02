/**
 * @file goptionsConventions.h
 * @brief Conventions, constants, and error codes for the \ref GOptions : / \ref GOption : subsystem.
 *
 * @details
 * This header collects:
 * - formatting constants for help output,
 * - the reserved option name used to persist version metadata,
 * - exit codes used by option parsing and validation,
 * - and the \ref goptions::NODFLT : marker that flags mandatory keys in structured options.
 *
 * @note
 * When writing documentation, if you need to mention the Doxygen command literally,
 * use `\\ref` (double backslash) so it prints as text rather than being parsed.
 */

#pragma once

/**
 * @brief Padding used when printing option/switch help.
 *
 * @details
 * This string controls the width of the left column used by the help output produced by
 * \ref GOptions : and \ref GOption : so the console formatting stays aligned.
 *
 * @note
 * The actual formatting is implemented in private methods inside those classes; private members
 * are intentionally not referenced here to avoid unresolved `\\ref` warnings when private extraction
 * is disabled in Doxygen.
 */
#define HELPFILLSPACE "                               "

/**
 * @brief Reserved option tag used to store version information.
 *
 * @details
 * \ref GOptions : registers this as a structured option so version metadata can be written to the saved YAML.
 * It is not intended to be set by users.
 */
#define GVERSION_STRING "version"

// exit codes: 100s
/**
 * @name Option parsing exit codes
 * @brief Process exit codes used by \ref GOptions : and \ref GOption : .
 *
 * @details
 * These codes are used in \c exit(code) calls to allow scripts/tests to distinguish failure modes.
 * @{
 */
#define EC__NOOPTIONFOUND                  101  ///< Option/switch/key not found or invalid command-line token.
#define EC__DEFINED_OPTION_ALREADY_PRESENT 102  ///< Attempted to define an option name more than once.
#define EC__DEFINED_SWITCHALREADYPRESENT   103  ///< Attempted to define a switch name more than once.
#define EC__YAML_PARSING_ERROR             104  ///< YAML file failed to parse (syntax or parser failure).
#define EC__MANDATORY_NOT_FILLED           105  ///< Mandatory structured option key (NODFLT) missing.
#define EC__BAD_CONVERSION                 106  ///< YAML value could not be converted to requested type.
#define EC_WRONG_VERBOSITY_LEVEL           107  ///< Verbosity/debug values outside accepted range (if enforced).
/** @} */


#include <string>

namespace goptions {
/**
 * @brief Marker literal indicating "no default value" for a structured option key.
 *
 * @details
 * When a \ref GVariable : in a structured option schema uses value \ref goptions::NODFLT : :
 * - that key becomes **mandatory** (must be provided by the user),
 * - and the option becomes **cumulative** (expects a sequence of maps).
 *
 * This allows a schema definition to encode both required fields and repeated entries.
 */
const std::string NODFLT = "NODFLT";
}
