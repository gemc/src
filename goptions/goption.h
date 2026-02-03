/**
 * @file goption.h
 * @brief Definitions of \ref GVariable : and \ref GOption : used by \ref GOptions : .
 */

#pragma once

// goption
#include "goptionsConventions.h"

// c++
#include <map>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

// yaml-cpp
#include "yaml-cpp/yaml.h"


/**
 * @struct GVariable
 * @brief Describes a schema entry: key name, default value, and user-facing description.
 *
 * @details
 * \ref GVariable : is primarily used when defining options:
 * - Scalar option: one \ref GVariable : holds option name, default value, and summary description.
 * - Structured option: a vector of \ref GVariable : entries defines the schema (keys and defaults).
 *
 * Values are stored internally as strings for uniform YAML construction and printing.
 */
struct GVariable
{
	std::string name; ///< Variable name (option name for scalar options, schema key name for structured options).
	std::string value; ///< Default value as a string (or \ref goptions::NODFLT : to mark as mandatory).
	std::string description; ///< Human-readable description used in help output.

	/**
	 * @brief Constructor for initializing a variable with a string default.
	 * @param n Name of the variable.
	 * @param val Default value (string) or \ref goptions::NODFLT : .
	 * @param d Description displayed in help.
	 */
	GVariable(std::string n, std::string val, std::string d)
		: name(std::move(n)), value(std::move(val)), description(std::move(d)) {
	}

	/**
	 * @brief Constructor for initializing a variable with a double default.
	 * @param n Name of the variable.
	 * @param val Default value (double), converted to string.
	 * @param d Description displayed in help.
	 */
	GVariable(std::string n, double val, std::string d)
		: name(std::move(n)), description(std::move(d)) { value = std::to_string(val); }

	/**
	 * @brief Constructor for initializing a variable with a C-string default.
	 * @param n Name of the variable.
	 * @param val Default value (C-string).
	 * @param d Description displayed in help.
	 */
	GVariable(std::string n, const char* val, std::string d)
		: name(std::move(n)), value(val), description(std::move(d)) {
	}

	/**
	 * @brief Constructor for initializing a variable with an integer default.
	 * @param n Name of the variable.
	 * @param val Default value (int), converted to string.
	 * @param d Description displayed in help.
	 */
	GVariable(std::string n, int val, std::string d)
		: name(std::move(n)), description(std::move(d)) { value = std::to_string(val); }

	/**
	 * @brief Constructor for initializing a variable with a boolean default.
	 * @param n Name of the variable.
	 * @param val Default value (bool), stored as "true" or "false".
	 * @param d Description displayed in help.
	 */
	GVariable(std::string n, bool val, std::string d)
		: name(std::move(n)), description(std::move(d)) { value = val ? "true" : "false"; }
};

/**
 * @class GOption
 * @brief Stores one configuration option (scalar or structured), including schema defaults and current value.
 *
 * @details
 * \ref GOption : encapsulates:
 * - the option name and user-facing description,
 * - a default YAML node constructed from the option definition,
 * - a current YAML node holding the resolved configuration value(s),
 * - structured schema metadata for printing detailed help.
 *
 * Structured options are defined by a vector of \ref GVariable : entries. If any schema entry uses
 * \ref goptions::NODFLT : as its default, that schema key becomes mandatory and the option becomes
 * **cumulative** (expects a YAML sequence of maps).
 *
 * \ref GOptions : is a friend and drives parsing and saving.
 */
class GOption
{
public:
	/**
	 * @brief Constructor for a scalar option with a default value.
	 *
	 * @details
	 * Builds YAML nodes of the form:
	 * \code{.yaml}
	 * <name>: <default>
	 * \endcode
	 *
	 * The constructed node is stored in both:
	 * - \c defaultValue (schema/default snapshot),
	 * - \c value (current resolved value), initially equal to the default.
	 *
	 * @param dv Scalar option descriptor (name, default value, description).
	 * @param h Multi-line help text shown in detailed help.
	 */
	GOption(GVariable dv, std::string h) : name(dv.name), description(dv.description), help(h) {
		defaultValue = YAML::Load(name + ": " + dv.value);
		value        = defaultValue;
	}

	/**
	 * @brief Constructor for a structured option schema (map/sequence).
	 *
	 * @details
	 * The schema is defined by @p dv, where each \ref GVariable : entry corresponds to one key.
	 * The default value node is constructed as a sequence of single-entry maps:
	 * \code{.yaml}
	 * optionName:
	 *   - key1: default1
	 *   - key2: default2
	 * \endcode
	 *
	 * If any schema entry has value \ref goptions::NODFLT : :
	 * - that key is added to \c mandatory_keys
	 * - the option is flagged cumulative (\c isCumulative = true)
	 *
	 * For non-cumulative structured options, the current \c value is initialized to the default schema.
	 * For cumulative options, the current \c value is intentionally left unset until user values are provided,
	 * because mandatory keys require explicit user input.
	 *
	 * @param n Option name.
	 * @param desc One-line description shown in summary help.
	 * @param dv Schema entries (keys, defaults, descriptions).
	 * @param h Multi-line help text shown in detailed help.
	 */
	GOption(std::string n, std::string desc, std::vector<GVariable> dv, std::string h)
		: name(n), description(desc), help(h) {
		YAML::Node nodes;
		for (const auto& v : dv) {
			YAML::Node this_node = YAML::Load(v.name + ": " + v.value);
			nodes.push_back(this_node);
			gvar_descs.push_back(v.description);
			if (v.value == goptions::NODFLT) {
				isCumulative = true;
				mandatory_keys.push_back(v.name);
			}
		}
		defaultValue[n] = nodes;
		if (!isCumulative) { value = defaultValue; }
	}

	/**
	 * @brief Updates a structured sub-option using dot-notation semantics.
	 *
	 * @details
	 * For a command-line payload like:
	 * \code{.sh}
	 * -verbosity.general=2
	 * \endcode
	 * \ref GOptions : locates the \ref GOption : for "verbosity" and calls this method to update
	 * the "general" entry.
	 *
	 * Update rules:
	 * - If the stored value is a YAML sequence of maps: every map entry that contains @p subkey is updated.
	 * - If the stored value is a YAML map: the map entry @p subkey is updated.
	 * - If @p subkey does not exist in the stored structured value, the program exits with \c EC__NOOPTIONFOUND .
	 *
	 * @param subkey The sub-option key to update.
	 * @param subvalue The new value expressed as a string; it is parsed as YAML.
	 */
	void set_sub_option_value(const std::string& subkey, const std::string& subvalue);

private:
	bool              isCumulative = false; ///< True for cumulative structured options (mandatory keys present in schema).
	const std::string name;                 ///< Option name (tag without leading '-'); this is the stable lookup key.
	const std::string description;          ///< Short summary description shown in one-line help output.
	const std::string help;                 ///< Multi-line help text shown in detailed help output (may include examples).

	YAML::Node               value;          ///< Current resolved YAML value for this option (after YAML + CLI parsing).
	YAML::Node               defaultValue;   ///< Default YAML value/schema for this option (constructed from definitions).
	std::vector<std::string> gvar_descs;     ///< Per-schema-key descriptions aligned with defaultValue sequence order.
	std::vector<std::string> mandatory_keys; ///< Keys that must be present in every cumulative entry.

	/**
	 * @brief Saves the current option value to a YAML configuration file.
	 *
	 * @details
	 * Serialization behavior:
	 * - Writes comment lines for any YAML null values (paths) to help users understand which fields were not provided.
	 * - Writes the YAML node in block style so nested maps/sequences remain readable.
	 *
	 * This method is invoked by the option manager during configuration snapshot writing.
	 *
	 * @param yamlConf Output stream for the YAML configuration file.
	 */
	void saveOption(std::ofstream* yamlConf) const;

	/**
	 * @brief Prints help text for this option.
	 *
	 * @details
	 * Output behavior:
	 * - Summary mode prints a single aligned line (name + short description).
	 * - Detailed mode additionally prints schema key descriptions and the full multi-line help text.
	 *
	 * This method is used by the option manager when generating console help output.
	 *
	 * @param detailed If true, prints schema defaults and extended help.
	 */
	void printHelp(bool detailed) const;

	/**
	 * @brief Builds detailed help text for this option.
	 *
	 * @details
	 * Detailed help includes:
	 * - the structured schema keys (if present) with their default values,
	 * - the per-key description strings,
	 * - and the free-form multi-line help text (often containing examples).
	 *
	 * @return Multi-line detailed help string.
	 */
	std::string detailedHelp() const;

	/**
	 * @brief Sets a scalar option value from a command-line string.
	 *
	 * @details
	 * The input string is normalized for compatibility with legacy comma-delimited payloads.
	 * The resulting string becomes the YAML scalar stored as the current option value.
	 *
	 * This method is called by the option manager while parsing scalar command-line tokens.
	 *
	 * @param v Input string value.
	 */
	void set_scalar_value(const std::string& v);

	/**
	 * @brief Sets the option value from a parsed YAML node.
	 *
	 * @details
	 * For cumulative structured options:
	 * - validates that each entry contains all mandatory keys,
	 * - stores the provided sequence of maps,
	 * - back-fills missing optional keys from schema defaults.
	 *
	 * For non-cumulative structured options:
	 * - updates existing structured content by matching keys and replacing values.
	 *
	 * This method is called by the option manager while parsing YAML files or structured command-line tokens.
	 *
	 * @param v Parsed YAML node.
	 */
	void set_value(const YAML::Node& v);

	/**
	 * @brief Checks whether all mandatory keys are present in a candidate YAML node.
	 *
	 * @details
	 * Mandatory keys are those whose schema default was \ref goptions::NODFLT : .
	 * The check is applied to each element of a cumulative sequence.
	 *
	 * @param v Candidate YAML node (typically one element of a sequence).
	 * @return True if all mandatory keys are present; false otherwise.
	 */
	bool does_the_option_set_all_necessary_values(const YAML::Node& v);

	friend class GOptions; ///< Grants \ref GOptions : access to private members for parsing and serialization.
};
