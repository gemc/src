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
 * @brief The GOptions class manages command-line options and switches.
 *
 * This class provides methods to define, access, and manage configuration options and switches
 * for command-line applications. Options can be scalar or sequence-based and are stored in YAML format.
 */
class GOptions {
public:
	/**
	 * @brief Default constructor.
	 *
	 * Creates an empty GOptions object, typically used for defining user–defined options.
	 */
	GOptions() {
	};

	/**
	 * @brief Constructor for verbosity and debug options.
	 *
	 * @param name: verbosity and debug option name
	 */
	explicit GOptions(std::string name) : option_verbosity_name(name) { addOptionTitle(std::move(name)); }

	/**
	 * @brief Library–based constructor.
	 *
	 * Constructs a GOptions object by initializing built–in options and switches,
	 * loading user-defined options, parsing YAML files, and processing command line arguments.
	 *
	 * @param argc Number of command–line arguments.
	 * @param argv Array of command–line argument strings.
	 * @param user_defined_options A GOptions object containing user–defined options.
	 */
	GOptions(int argc, char* argv[], const GOptions& user_defined_options = GOptions());


	~GOptions() {
		if (yamlConf != nullptr) {
			if (yamlConf->is_open()) { yamlConf->close(); }
			delete yamlConf;
			yamlConf = nullptr;
		}
	}

	/**
	 * @brief Defines and adds a command–line switch.
	 * @param name The name of the switch.
	 * @param description The description of the switch.
	 */
	void defineSwitch(const std::string& name, const std::string& description);

	/**
	 * @brief Defines and adds a scalar option.
	 * @param gvar A GVariable representing the option.
	 * @param help Help text for the option.
	 */
	void defineOption(const GVariable& gvar, const std::string& help);

	/**
	 * @brief Defines and adds a structured (map/sequence) option.
	 * @param name The name of the option.
	 * @param description A summary description of the option.
	 * @param gvars A vector of GVariable objects representing the option elements.
	 * @param help Help text for the option.
	 */
	void defineOption(const std::string& name, const std::string& description, const std::vector<GVariable>& gvars,
	                  const std::string& help);

	/**
	 * @brief Retrieves the value of a scalar integer option.
	 * @param tag The name of the option.
	 * @return The integer value.
	 */
	[[nodiscard]] int getScalarInt(const std::string& tag) const;


	/**
	 * @brief Retrieves the value of a scalar double option.
	 * @param tag The name of the option.
	 * @return The double value.
	 */
	[[nodiscard]] double getScalarDouble(const std::string& tag) const;

	/**
	 * @brief Retrieves the value of a scalar string option.
	 * @param tag The name of the option.
	 * @return The string value.
	 */
	[[nodiscard]] std::string getScalarString(const std::string& tag) const;

	/**
	 * @brief Retrieves the status of a switch.
	 * @param tag The name of the switch.
	 * @return True if the switch is on; false otherwise.
	 */
	[[nodiscard]] bool getSwitch(const std::string& tag) const;

	/**
	 * @brief Retrieves the YAML node for the specified option.
	 *
	 * @param tag The name of the option.
	 * @return The YAML::Node containing the option's value.
	 */
	[[nodiscard]] inline YAML::Node getOptionNode(const std::string& tag) const {
		if (!doesOptionExist(tag)) {
			std::cerr << "Option " << tag << " does not exist. Exiting." << std::endl;
			exit(EC__NOOPTIONFOUND);
		}
		return getOptionIterator(tag)->value.begin()->second;
	}

	/**
	 * @brief Retrieves a map option’s value from within a YAML node.
	 * @param option_name The name of the option.
	 * @param map_key The key within the option.
	 * @return The YAML::Node corresponding to the specified map key.
	 */
	[[nodiscard]] YAML::Node getOptionMapInNode(const std::string& option_name, const std::string& map_key) const;

	/**
	 * @brief Retrieves the verbosity level for the specified tag.
	 * @param tag The name of the verbosity option.
	 * @return The verbosity level as an integer.
	 */
	[[nodiscard]] int getVerbosityFor(const std::string& tag) const;

	/**
	 * @brief Retrieves the debug level for the specified tag.
	 *
	 * Accepts values as either booleans ("true"/"false") or integers.
	 *
	 * @param tag The name of the debug option.
	 * @return The debug level as an integer.
	 */
	[[nodiscard]] int getDebugFor(const std::string& tag) const;

	/**
	 * @brief Returns the list of defined options.
	 * @return A vector of GOption objects.
	 */
	[[nodiscard]] const std::vector<GOption>& getOptions() const { return goptions; }

	/**
	 * @brief Returns the map of defined switches.
	 * @return A map of switches.
	 */
	[[nodiscard]] const std::map<std::string, GSwitch>& getSwitches() const { return switches; }

	/**
	 * @brief Adds options from another GOptions object.
	 * @param src The GOptions object to add.
	 */
	inline void addGOptions(const GOptions& src)
	{
		// 1.  Options – check by option name
		for (const auto& opt : src.getOptions()) {
			auto already = std::find_if(
				goptions.begin(), goptions.end(),
				[&opt](const GOption& o) { return o.name == opt.name; });

			if (already == goptions.end())
				goptions.push_back(opt);             // add only if absent
		}

		// 2.  Switches – std::map::insert does the uniqueness check for us
		for (const auto& sw : src.getSwitches()) {
			switches.insert(sw);                     // ignored if key already exists
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

	std::string            option_verbosity_name{UNINITIALIZEDSTRINGQUANTITY};
	std::vector<GVariable> option_verbosity_names;

	inline void addOptionTitle(const std::string& name) {
		std::string option_verbosity_name_desc = name + " verbosity level or debug switch";
		option_verbosity_names.emplace_back(name, 0, option_verbosity_name_desc);
	}

	/**
	 * @brief Retrieves a variable from a YAML node within an option.
	 * @tparam T The type of the variable.
	 * @param node The YAML node.
	 * @param variable_name The name of the variable.
	 * @param default_value The default value if the variable is not found.
	 * @return The variable value.
	 */
	template <typename T>
	T get_variable_in_option(const YAML::Node& node, const std::string& variable_name, const T& default_value);

	/**
	 * @brief Retrieves the list of YAML file paths.
	 * @return A vector of YAML file paths as strings.
	 */
	[[nodiscard]] std::vector<std::string> getYamlFiles() const { return yaml_files; }

	/**
	 * @brief Checks if the specified option exists.
	 * @param tag The name of the option.
	 * @return True if the option exists; false otherwise.
	 */
	[[nodiscard]] bool doesOptionExist(const std::string& tag) const;

private:
	std::vector<GOption>           goptions;             ///< Array of GOption objects.
	std::map<std::string, GSwitch> switches;             ///< Map of GSwitch objects.
	std::ofstream*                 yamlConf{};           ///< YAML configuration file stream.
	std::string                    executableName;       ///< Executable name.
	std::string                    executableCallingDir; ///< Executable calling dir
	std::string                    installDir;           ///< Executable installation dir
	std::vector<std::string>       yaml_files;           ///< List of YAML file paths.

	/**
	 * @brief Finds YAML files specified by the command line.
	 * @param argc Number of command-line arguments.
	 * @param argv Array of command-line argument strings.
	 * @return A vector of YAML file paths.
	 */
	std::vector<std::string> findYamls(int argc, char* argv[]);

	/**
	 * @brief Parses and sets option values from a YAML file.
	 * @param yaml The YAML file path.
	 */
	void setOptionsValuesFromYamlFile(const std::string& yaml);

	/**
	 * @brief Parses and sets option values from a command-line argument.
	 * @param optionName The name of the option.
	 * @param possibleYamlNode The YAML-formatted value string.
	 */
	void setOptionValuesFromCommandLineArgument(const std::string& optionName, const std::string& possibleYamlNode);

	/**
	 * @brief Retrieves an iterator to the option with the specified name.
	 * @param name The name of the option.
	 * @return An iterator to the option.
	 */
	std::vector<GOption>::iterator getOptionIterator(const std::string& name);

	/**
	 * @brief Retrieves a const iterator to the option with the specified name.
	 * @param name The name of the option.
	 * @return A const iterator to the option.
	 */
	[[nodiscard]] std::vector<GOption>::const_iterator getOptionIterator(const std::string& name) const;

	/**
	 * @brief Prints help information for a specific option or switch.
	 * @param tag The name of the option or switch.
	 */
	void printOptionOrSwitchHelp(const std::string& tag) const;

	/**
	 * @brief Prints general help information to the console.
	 */
	void printHelp() const;

	/**
	 * @brief Prints web-formatted help information.
	 */
	void printWebHelp() const;

	/**
	 * @brief Saves all user options to a YAML configuration file.
	 */
	void saveOptions() const;

	/**
	 * @brief Prints the version information of the executable.
	 */
	void print_version();
};

/// Overloaded operator to add options and switches from one GOptions object to another.
GOptions& operator+=(GOptions& original, const GOptions& optionsToAdd);
