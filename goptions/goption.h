#ifndef GOPTION_H
#define GOPTION_H 1

// goption
#include "goptionsConventions.h"
// gemc
#include "gutsConventions.h"
// c++
#include <map>
#include <fstream>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>

// yaml-cpp
#include "yaml-cpp/yaml.h"

using std::string;
using std::map;
using std::vector;

/**
 * @struct GVariable
 * @brief Encapsulates a variable with a name, value, and description.
 *
 * The GVariable struct provides a flexible way to store and manage different types of
 * variables, with overloaded constructors to support strings, integers, doubles, and booleans.
 */
struct GVariable {
	string name;        ///< The name of the variable.
	string value;       ///< The value of the variable, stored as a string.
	string description; ///< A brief description of the variable.

	/**
	 * @brief Constructor for initializing a variable with a string value.
	 * @param n Name of the variable.
	 * @param val Value of the variable.
	 * @param d Description of the variable.
	 */
	GVariable(string n, string val, string d)
			: name(std::move(n)), value(std::move(val)), description(std::move(d)) {}

	/**
	 * @brief Constructor for initializing a variable with a double value.
	 * @param n Name of the variable.
	 * @param val Value of the variable.
	 * @param d Description of the variable.
	 */
	GVariable(string n, double val, string d)
			: name(std::move(n)), description(std::move(d)) { value = std::to_string(val); }

	/**
	 * @brief Constructor for initializing a variable with a const char* value.
	 * @param n Name of the variable.
	 * @param val Value of the variable.
	 * @param d Description of the variable.
	 */
	GVariable(string n, const char *val, string d)
			: name(std::move(n)), value(val), description(std::move(d)) {}

	/**
	 * @brief Constructor for initializing a variable with an integer value.
	 * @param n Name of the variable.
	 * @param val Value of the variable.
	 * @param d Description of the variable.
	 */
	GVariable(string n, int val, string d)
			: name(std::move(n)), description(std::move(d)) { value = std::to_string(val); }

	/**
	 * @brief Constructor for initializing a variable with a boolean value.
	 * @param n Name of the variable.
	 * @param val Boolean value of the variable.
	 * @param d Description of the variable.
	 */
	GVariable(string n, bool val, string d)
			: name(std::move(n)), description(std::move(d)) { value = val ? "true" : "false"; }
};

/**
 * @class GOption
 * @brief Represents a configurable option with a name, value(s), description, and help text.
 *
 * The GOption class is used to define options from a YAML configuration node. Options can be scalar or sequences,
 * and they can have default values or be cumulative. This class encapsulates the logic to set values from YAML nodes
 * and command-line arguments.
 */
class GOption {
public:
	/**
	 * @brief Constructor for a scalar option with a default value.
	 * @param dv A GVariable containing the name, default value, and description of the option.
	 * @param h Help text for the option.
	 */
	GOption(GVariable dv, string h) : name(dv.name), description(dv.description), help(h) {
		defaultValue = YAML::Load(name + ": " + dv.value);
		value = defaultValue;
	}

	/**
	 * @brief Constructor for a sequence option.
	 * @param n Name of the option.
	 * @param desc Summary description of the option.
	 * @param dv Vector of GVariable objects representing each element's default value and description.
	 * @param h Help text for the option.
	 */
	GOption(string n, string desc, vector<GVariable> dv, string h)
			: name(n), description(desc), help(h) {
		YAML::Node nodes;
		for (const auto &v : dv) {
			YAML::Node this_node = YAML::Load(v.name + ": " + v.value);
			nodes.push_back(this_node);
			gvar_descs.push_back(v.description);
			if (v.value == goptions::NODFLT) {
				isCumulative = true;
				mandatory_keys.push_back(v.name);
			}
		}
		defaultValue[n] = nodes;
		if (!isCumulative) {
			value = defaultValue;
		}
	}

	/**
	 * @brief Sets the value of a sub–option using dot–notation.
	 *
	 * For example, a command-line argument like "-verbosity.general=2" will update the sub–option
	 * identified by "general" within the "verbosity" option.
	 *
	 * @param subkey The sub–option key to update.
	 * @param subvalue The new value for the sub–option, given as a string.
	 */
	void set_sub_option_value(const string &subkey, const string &subvalue);

private:
	bool isCumulative = false;          ///< Indicates whether the option is cumulative (sequence).
	const string name;                  ///< The name of the option.
	const string description;           ///< A summary description of the option.
	const string help;                  ///< Help text for the option.

	YAML::Node value;                   ///< The current value(s) of the option.
	YAML::Node defaultValue;            ///< The default value(s) of the option.
	vector<string> gvar_descs;          ///< Descriptions for each element in a sequence option.
	vector<string> mandatory_keys;      ///< Keys that must be provided for a valid option.

	/**
	 * @brief Saves the current option value to a YAML configuration file.
	 * @param yamlConf Pointer to the output file stream.
	 */
	void saveOption(std::ofstream *yamlConf) const;

	/**
	 * @brief Prints the help information for the option.
	 * @param detailed If true, prints detailed help; otherwise, prints a summary.
	 */
	void printHelp(bool detailed) const;

	/**
	 * @brief Returns detailed help text for the option.
	 * @return A string containing detailed help information.
	 */
	string detailedHelp() const;

	/**
	 * @brief Sets the value of a scalar option based on a command-line string.
	 * @param v The input string to be set as the option’s value.
	 */
	void set_scalar_value(const string &v);

	/**
	 * @brief Sets the option value based on a parsed YAML node.
	 * @param v The YAML node containing the new value(s).
	 */
	void set_value(const YAML::Node &v);

	/**
	 * @brief Checks if all mandatory keys (marked with NODFLT) are present in the YAML node.
	 * @param v The YAML node to check.
	 * @return True if all mandatory keys are present; false otherwise.
	 */
	bool does_the_option_set_all_necessary_values(const YAML::Node& v);

	friend class GOptions; ///< Grants GOptions access to private members.
};

#endif
