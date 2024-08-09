#ifndef  GOPTIONS_H
#define  GOPTIONS_H  1

#include "goption.h"
#include "gswitch.h"

// c++
#include <string>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>

/**
 * @brief The GOptions class manages command-line options and switches.
 *
 * This class provides methods to define, access, and manage configuration options
 * and switches for command-line applications. Options can be scalar or sequence-based
 * and are stored in YAML format.
 */

class GOptions {

public:

    /**
     * @brief Default constructor.
     *
     * Creates an empty GOptions object, typically used for defining user-defined options.
     */
    GOptions() { return; }

    /**
     * @details Library based constructor: each library can define its own options,
     * \param argc number of arguments, passed from "main"
     * \param argv argument arrays of *chars, passed from main
     * \param user_defined_options vector of user options, usually returned by a defineOptions() function
     */
    GOptions(int argc, char *argv[], const GOptions &user_defined_options);

    /**
     * @brief Defines and adds a command-line switch to the map of switches.
     *
     * @param name The name of the switch.
     * @param description The description of the switch.
     */
    void defineSwitch(const std::string &name, const std::string &description);

    /**
     * @brief Defines and adds a scalar option to the map of options.
     *
     * @param gvar The GVariable object representing the scalar option.
     * @param help The help message for the option.
     */
    void defineOption(const GVariable &gvar, const std::string &help);

    /**
     * @brief Defines and adds a structured option to the map of options.
     *
     * @param name The name of the option.
     * @param description The description of the option.
     * @param gvars A vector of GVariable objects representing the structured option.
     * @param help The help message for the option.
     */
    void defineOption(const std::string &name, const std::string &description, const std::vector <GVariable> &gvars, const std::string &help);

    /**
     * @brief Retrieves the value of a scalar integer option.
     *
     * @param tag The name of the option.
     * @return The integer value of the option.
     */
    int getScalarInt(const std::string &tag) const;

    /**
     * @brief Retrieves the value of a scalar float option.
     *
     * @param tag The name of the option.
     * @return The float value of the option.
     */
    float getScalarFloat(const std::string &tag) const;

    /**
     * @brief Retrieves the value of a scalar double option.
     *
     * @param tag The name of the option.
     * @return The double value of the option.
     */
    double getScalarDouble(const std::string &tag) const;

    /**
     * @brief Retrieves the value of a scalar string option.
     *
     * @param tag The name of the option.
     * @return The string value of the option.
     */
    std::string getScalarString(const std::string &tag) const;

    /**
     * @brief Retrieves the status of a switch.
     *
     * @param tag The name of the switch.
     * @return The boolean status of the switch.
     */
    bool getSwitch(const std::string &tag) const;

    /**
     * @brief Retrieves the YAML::Node of the specified option.
     *
     * @param tag The name of the option.
     * @return The YAML::Node containing the option's value.
     */
    inline const YAML::Node getOptionNode(const std::string &tag) const {
        // If the option does not exist, exit with error
        if (!doesOptionExist(tag)) {
            std::cerr << "Option " << tag << " does not exist. Exiting." << std::endl;
            gexit(EC__NOOPTIONFOUND);
        }
        return getOptionIterator(tag)->value.begin()->second;
    }

    /**
      * @brief Retrieves a map option within a YAML::Node.
      *
      * @param option_name The name of the option.
      * @param map_key The key of the map within the option.
      * @return The YAML::Node corresponding to the map key.
      */
    YAML::Node getOptionMapInNode(string option_name, string map_key);

    /**
     * @brief Retrieves the verbosity level for a given tag.
     *
     * @param tag The name of the verbosity option.
     * @return The verbosity level as an integer.
     */
    int getVerbosityFor(const std::string &tag) const;

    /**
     * @brief Returns the list of all defined options.
     *
     * @return A vector of GOption objects.
     */
    const std::vector <GOption> &getOptions() const { return goptions; }

    /**
     * @brief Returns the map of all defined switches.
     *
     * @return A map of switches.
     */
    const std::map <std::string, GSwitch> &getSwitches() const { return switches; }

    /**
     * @brief Adds a set of GOptions to the current options.
     *
     * @param goptions_to_add The GOptions object to be added.
     */
    inline void addGOptions(const GOptions &goptions_to_add) {
        for (auto gopt: goptions_to_add.getOptions()) {
            goptions.push_back(gopt);
        }
        for (auto sw: goptions_to_add.getSwitches()) {
            switches.insert(sw);
        }
    }

    /**
     * @brief Retrieves a variable from a YAML::Node.
     *
     * @param node The YAML::Node containing the variable.
     * @param variable_name The name of the variable.
     * @param default_value The default value to return if the variable is not found.
     * @return The value of the variable.
     */
    template<typename T>
    T get_variable_in_option(const YAML::Node &node, const string &variable_name, const T &default_value);

    /**
     * @brief Retrieves the list of YAML files.
     *
     * @return A vector of YAML file paths as strings.
     */
    std::vector <std::string> getYamlFiles() const { return yaml_files; }

private:

    std::vector <GOption> goptions;           ///< Array of GOption objects.
    std::map <std::string, GSwitch> switches; ///< Map of GSwitch objects.
    std::ofstream *yamlConf;                 ///< YAML configuration file stream.
    std::string executableName;              ///< Name of the executable.
    std::vector <std::string> yaml_files;     ///< List of YAML files.


    /**
     * @brief Finds YAML files specified by the command line.
     *
     * @param argc Number of command-line arguments.
     * @param argv Array of command-line arguments.
     * @return A vector of YAML file paths as strings.
     */
    vector <string> findYamls(int argc, char *argv[]);  // finds the yaml specified by command line. Returns "na' if not found.

    /**
     * @brief Parses and sets option values from a YAML file.
     *
     * @param yaml The path to the YAML file.
     */
    void setOptionsValuesFromYamlFile(const std::string &yaml);

    /**
     * @brief Parses and sets option values from a command-line argument.
     *
     * @param optionName The name of the option.
     * @param possibleYamlNode The potential YAML node.
     */
    void setOptionValuesFromCommandLineArgument(const std::string &optionName, const std::string &possibleYamlNode);

    /**
     * @brief Checks if the specified option exists.
     *
     * @param tag The name of the option.
     * @return True if the option exists, false otherwise.
     */
    bool doesOptionExist(const std::string &tag) const;

    /**
     * @brief Retrieves the iterator to an option with the specified name.
     *
     * @param name The name of the option.
     * @return An iterator to the option.
     */
    std::vector<GOption>::iterator getOptionIterator(const std::string &name);

    std::vector<GOption>::const_iterator getOptionIterator(const std::string &name) const;

    /**
     * @brief Prints help information for a single option or switch.
     *
     * @param tag The name of the option or switch.
     */
    void printOptionOrSwitchHelp(const std::string &tag) const;

    /**
     * @brief Loops over all options and prints help information.
     */
    void printHelp() const;

    /**
     * @brief Prints web-formatted help information.
     */
    void printWebHelp() const;

    /**
     * @brief Saves all user options in a YAML file.
     */
    void saveOptions() const;

    /**
     * @brief Prints the version of the executable.
     */
    void print_version();


};

// overloaded operator to add option vectors and switch maps
GOptions &operator+=(GOptions &original, const GOptions &optionsToAdd);

#endif
