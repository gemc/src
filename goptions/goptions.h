/**
 * @file goptions.h
 * @brief Public interface for \ref GOptions : the YAML + command-line configuration manager.
 */

#pragma once

#include "goption.h"
#include "gswitch.h"

// gemc
#include "gutsConventions.h"


// c++
#include <string>
#include <fstream>
#include <iostream>
#include <map>
#include <utility>
#include <vector>

/**
 * @class GOptions
 * @brief Parses, stores, and exposes command-line options and YAML configuration values.
 *
 * @details
 * \ref GOptions : manages two categories of configuration inputs:
 * - **Switches** (\ref GSwitch :): boolean flags toggled by presence (e.g., `-gui`).
 * - **Options** (\ref GOption :): values that can be scalar or structured (map/sequence),
 *   set via YAML files and/or `-name=value` command line arguments.
 *
 * Parsing precedence is:
 * 1. YAML file(s) (in the order they appear on the command line)
 * 2. Command-line options, which overwrite YAML values
 *
 * The main constructor \ref GOptions::GOptions "GOptions(argc, argv, user_defined_options)" performs parsing
 * immediately and may call \c exit() for help/version requests or invalid inputs.
 */
class GOptions
{
public:
	/**
	 * @brief Default constructor.
	 *
	 * @details
	 * Creates an empty \ref GOptions : instance. This is primarily used to build up
	 * user-defined option sets (e.g., in plugin/framework `defineOptions()` functions)
	 * that will later be merged into a parsing instance.
	 */
	GOptions() {
	};

	/**
	 * @brief Constructor for creating verbosity/debug schema helpers.
	 *
	 * @details
	 * This convenience constructor registers one schema entry (via \c addOptionTitle())
	 * into \c option_verbosity_names. It is used to populate the standard `verbosity` and `debug`
	 * structured options.
	 *
	 * @param name The verbosity/debug key name to add to the schema.
	 */
	explicit GOptions(std::string name) : option_verbosity_name(name) { addOptionTitle(std::move(name)); }

	/**
	 * @brief Main constructor: registers options and parses inputs (YAML + command line).
	 *
	 * @details
	 * See implementation documentation in `goptions.cc` for the full parsing pipeline.
	 *
	 * @param argc Number of command-line arguments.
	 * @param argv Array of command-line argument strings.
	 * @param user_defined_options A \ref GOptions : containing additional options/switches to register.
	 */
	GOptions(int argc, char* argv[], const GOptions& user_defined_options = GOptions());


	/**
	 * @brief Destructor.
	 *
	 * @details
	 * Owns and deletes \c yamlConf if it was allocated by the parsing constructor.
	 * This ensures file handles are closed and memory is released deterministically.
	 */
	~GOptions() {
		if (yamlConf != nullptr) {
			if (yamlConf->is_open()) { yamlConf->close(); }
			delete yamlConf;
			yamlConf = nullptr;
		}
	}

	/**
	 * @brief Defines and adds a command-line switch.
	 *
	 * @details
	 * Switches are presence-based boolean flags (default off) and are activated by
	 * specifying `-<name>` on the command line.
	 *
	 * @param name Switch name (without leading '-').
	 * @param description Text shown in help output.
	 */
	void defineSwitch(const std::string& name, const std::string& description);

	/**
	 * @brief Defines and adds a scalar option.
	 *
	 * @details
	 * Scalar options hold a single value (int/double/string/bool expressed as string in YAML)
	 * and are set using `-name=value`.
	 *
	 * @param gvar Option descriptor (name, default value, description).
	 * @param help Additional detailed help text.
	 */
	void defineOption(const GVariable& gvar, const std::string& help);

	/**
	 * @brief Defines and adds a structured option (map or sequence of maps).
	 *
	 * @details
	 * A structured option is described by a vector of \ref GVariable : schema entries.
	 * If any schema entry uses \ref goptions::NODFLT : as its default, that key becomes mandatory
	 * and the option becomes cumulative.
	 *
	 * @param name Option name (without leading '-').
	 * @param description Brief description shown in summary help.
	 * @param gvars Schema definitions (keys, defaults, descriptions).
	 * @param help Detailed help text and examples.
	 */
	void defineOption(const std::string& name, const std::string& description, const std::vector<GVariable>& gvars,
	                  const std::string& help);

	/**
	 * @brief Retrieves the value of a scalar integer option.
	 *
	 * @details
	 * See \ref GOptions::getScalarString "getScalarString()" for YAML null sentinel behavior.
	 *
	 * @param tag Option name.
	 * @return Value converted to int.
	 */
	[[nodiscard]] int getScalarInt(const std::string& tag) const;


	/**
	 * @brief Retrieves the value of a scalar double option.
	 *
	 * @param tag Option name.
	 * @return Value converted to double.
	 */
	[[nodiscard]] double getScalarDouble(const std::string& tag) const;

	/**
	 * @brief Retrieves the value of a scalar string option.
	 *
	 * @details
	 * If the underlying YAML node is null, returns the literal sentinel `"NULL"`.
	 *
	 * @param tag Option name.
	 * @return Value as string (or `"NULL"`).
	 */
	[[nodiscard]] std::string getScalarString(const std::string& tag) const;

	/**
	 * @brief Retrieves the status of a switch.
	 *
	 * @param tag Switch name.
	 * @return True if enabled; false otherwise.
	 */
	[[nodiscard]] bool getSwitch(const std::string& tag) const;

	/**
	 * @brief Retrieves the YAML node for the specified option.
	 *
	 * @details
	 * This provides direct access to the YAML node underlying the option, enabling clients to
	 * inspect structured content without additional copying.
	 *
	 * @param tag Option name.
	 * @return YAML node representing the option value.
	 *
	 * @warning
	 * If the option does not exist, this function exits with \c EC__NOOPTIONFOUND .
	 */
	[[nodiscard]] inline YAML::Node getOptionNode(const std::string& tag) const {
		if (!doesOptionExist(tag)) {
			std::cerr << "Option " << tag << " does not exist. Exiting." << std::endl;
			exit(EC__NOOPTIONFOUND);
		}
		return getOptionIterator(tag)->value.begin()->second;
	}

	/**
	 * @brief Retrieves a map entry value from a structured option stored as a sequence of maps.
	 *
	 * @details
	 * This is commonly used for options like `verbosity` and `debug`, where each sequence element
	 * is a `{key: value}` map.
	 *
	 * @param option_name Structured option name.
	 * @param map_key Key to retrieve.
	 * @return YAML node associated with @p map_key .
	 */
	[[nodiscard]] YAML::Node getOptionMapInNode(const std::string& option_name, const std::string& map_key) const;

	/**
	 * @brief Retrieves the verbosity level for the specified tag.
	 *
	 * @param tag Verbosity key (e.g., "ghits").
	 * @return Verbosity level as integer.
	 */
	[[nodiscard]] int getVerbosityFor(const std::string& tag) const;

	/**
	 * @brief Retrieves the debug level for the specified tag.
	 *
	 * @details
	 * Accepts values as either booleans ("true"/"false") or integers.
	 *
	 * @param tag Debug key (e.g., "general").
	 * @return Debug level as integer.
	 */
	[[nodiscard]] int getDebugFor(const std::string& tag) const;

	/**
	 * @brief Returns the list of defined options.
	 *
	 * @return Const reference to internal option vector.
	 */
	[[nodiscard]] const std::vector<GOption>& getOptions() const { return goptions; }

	/**
	 * @brief Returns the map of defined switches.
	 *
	 * @return Const reference to internal switch map.
	 */
	[[nodiscard]] const std::map<std::string, GSwitch>& getSwitches() const { return switches; }

	/**
	 * @brief Merges options and switches from another \ref GOptions : into this one.
	 *
	 * @details
	 * Merge rules:
	 * - **Options** are merged by option name; an option is added only if absent.
	 * - **Switches** are inserted by name; existing names remain unchanged.
	 * - **Verbosity/debug schema names** are merged by key name; duplicates are avoided.
	 *
	 * This function enables a plugin/framework pattern:
	 * \code{.cpp}
	 * goptions += pluginA::defineOptions();
	 * goptions += pluginB::defineOptions();
	 * \endcode
	 *
	 * @param src Source \ref GOptions : to merge into this instance.
	 */
	inline void addGOptions(const GOptions& src) {
		// 1.  Options – check by option name
		for (const auto& opt : src.getOptions()) {
			auto already = std::find_if(
				goptions.begin(), goptions.end(),
				[&opt](const GOption& o) { return o.name == opt.name; });

			if (already == goptions.end())
				goptions.push_back(opt); // add only if absent
		}

		// 2.  Switches – std::map::insert does the uniqueness check for us
		for (const auto& sw : src.getSwitches()) {
			switches.insert(sw); // ignored if key already exists
		}

		// 3.  Verbosity/debug variable names – store each only once
		for (const auto& v : src.option_verbosity_names) {
			auto same = std::find_if(
				option_verbosity_names.begin(), option_verbosity_names.end(),
				[&v](const GVariable& existing) { return existing.name == v.name; });

			if (same == option_verbosity_names.end())
				option_verbosity_names.push_back(v);
		}
	}

	/**
	 * @brief Name used when constructing the verbosity/debug schema helper.
	 *
	 * @details
	 * Defaults to \c UNINITIALIZEDSTRINGQUANTITY and is typically set by the
	 * \ref GOptions::GOptions(std::string) "GOptions(name)" helper constructor.
	 */
	std::string option_verbosity_name{UNINITIALIZEDSTRINGQUANTITY};

	/**
	 * @brief Schema entries used to define the `verbosity` and `debug` structured options.
	 *
	 * @details
	 * Each \ref GVariable : in this vector is used as a schema key (e.g., "general", "ghits", etc.)
	 * with an integer default value and a short description.
	 */
	std::vector<GVariable> option_verbosity_names;

	inline void addOptionTitle(const std::string& name) {
		std::string option_verbosity_name_desc = name + " verbosity level or debug switch";
		option_verbosity_names.emplace_back(name, 0, option_verbosity_name_desc);
	}

	/**
	 * @brief Retrieves a typed variable from a YAML node within an option.
	 *
	 * @tparam T The type of the variable.
	 * @param node YAML node to query.
	 * @param variable_name Key name.
	 * @param default_value Fallback value when key is absent.
	 * @return Parsed value or fallback.
	 */
	template <typename T>
	T get_variable_in_option(const YAML::Node& node, const std::string& variable_name, const T& default_value);

	/**
	 * @brief Returns the list of YAML file paths detected on the command line.
	 *
	 * @return Vector of YAML file path strings.
	 */
	[[nodiscard]] std::vector<std::string> getYamlFiles() const { return yaml_files; }

	/**
	 * @brief Checks if an option exists.
	 *
	 * @param tag Option name.
	 * @return True if the option exists; false otherwise.
	 */
	[[nodiscard]] bool doesOptionExist(const std::string& tag) const;

private:
	std::vector<GOption>           goptions;             ///< Registered options (scalar and structured).
	std::map<std::string, GSwitch> switches;             ///< Registered switches (boolean flags).
	std::ofstream*                 yamlConf{};           ///< Output stream for saved YAML configuration (owned).
	std::string                    executableName;       ///< Name of the executable (derived from argv[0]).
	std::string                    executableCallingDir; ///< Directory from which the executable was invoked.
	std::string                    installDir;           ///< Installation directory for the executable (gemc root).
	std::vector<std::string>       yaml_files;           ///< YAML configuration files detected on command line.

	std::vector<std::string> findYamls(int argc, char* argv[]);
	void setOptionsValuesFromYamlFile(const std::string& yaml);
	void setOptionValuesFromCommandLineArgument(const std::string& optionName, const std::string& possibleYamlNode);
	std::vector<GOption>::iterator getOptionIterator(const std::string& name);
	[[nodiscard]] std::vector<GOption>::const_iterator getOptionIterator(const std::string& name) const;
	void printOptionOrSwitchHelp(const std::string& tag) const;
	void printHelp() const;
	void printWebHelp() const;
	void saveOptions() const;
	void print_version();
};

/// Overloaded operator to add options and switches from one \ref GOptions : to another.
GOptions& operator+=(GOptions& original, const GOptions& optionsToAdd);
