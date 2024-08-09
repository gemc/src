#ifndef  GOPTION_H
#define  GOPTION_H  1

// goption
#include "goptionsConventions.h"

// gemc
#include "gutsConventions.h"

// c++
#include <map>
#include <fstream>
#include <vector>
#include <string>

using std::string;
using std::map;
using std::vector;

// yaml-cpp
#include "yaml-cpp/yaml.h"

/**
 * @struct GVariable
 * @brief Encapsulates a variable with a name, value, and description.
 *
 * The `GVariable` struct provides a flexible way to store and manage different
 * types of variables, allowing for overloading constructors to handle various
 * data types.
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
    GVariable(string n, string val, string d) : name(n), value(val), description(d) {}

    /**
     * @brief Constructor for initializing a variable with a string value.
     * @param n Name of the variable.
     * @param val Value of the variable.
     * @param d Description of the variable.
     */
    GVariable(string n, double val, string d) : name(n), description(d) { value = std::to_string(val); }

    /**
     * @brief Constructor for initializing a variable with an integer value.
     * @param n Name of the variable.
     * @param val Value of the variable.
     * @param d Description of the variable.
     */
    GVariable(string n, const char *val, string d) : name(n), value(val), description(d) {}

    /**
     * @brief Constructor for initializing a variable with a double value.
     * @param n Name of the variable.
     * @param val Value of the variable.
     * @param d Description of the variable.
     */
    GVariable(string n, int val, string d) : name(n), description(d) { value = std::to_string(val); }

    /**
     * @brief Constructor for initializing a variable with a boolean value.
     * @param n Name of the variable.
     * @param val Value of the variable.
     * @param d Description of the variable.
     */
    GVariable(string n, bool val, string d) : name(n), description(d) { value = val ? "true" : "false"; }
};

/**
 * @class GOption
 * @brief Represents a configurable option with a name, value, description, and help text.
 *
 * The `GOption` class is used to define options from a YAML configuration node.
 * Options can be scalar or sequences of values, and can have default values or be cumulative.
 */
class GOption {
public:

    /**
     * @brief Constructor for a scalar option with a default value.
     * @param dv A `GVariable` containing the name, default value, and description of the option.
     * @param h Help text for the option.
     *
     * This constructor initializes a scalar option using a string as the default value.
     * Scalar options cannot have a default value of `NODFLT`.
     */

    GOption(GVariable dv, string h) : name(dv.name), description(dv.description), help(h) {
        defaultValue = YAML::Load(name + ": " + dv.value);
        value = defaultValue;
    }


    /**
     * @brief Constructor for a sequence option.
     * @param n Name of the option.
     * @param desc Summary description of the option.
     * @param dv Vector of `GVariable` objects, each containing the name, default value, and description for elements in the sequence.
     * @param h Help text for the option.
     *
     * This constructor initializes a sequence option. If any of the default values is `NODFLT`, the option is cumulative;
     * otherwise, it is not. If the option is not cumulative, the default value is applied to the first element of the sequence.
     */
    GOption(string n, string desc, vector <GVariable> dv, string h) : name(n), description(desc), help(h) {

        YAML::Node nodes;
        for (const auto &v: dv) {
            YAML::Node this_node = YAML::Load(v.name + ": " + v.value);
            nodes.push_back(this_node);
            gvar_descs.push_back(v.description);
            if (v.value == goptions::NODFLT) {
                isCumulative = true;
                mandatory_keys.push_back(v.name);
            }
        }
        defaultValue[n] = nodes;

        // if an option is not cumulative, the default value is copied to the first element of the sequence
        if (!isCumulative) {
            value = defaultValue;
        }
    }

private:

    bool isCumulative = false;          ///< Indicates whether the option is cumulative.
    const string name;                  ///< The name of the option.
    const string description;           ///< A summary description of the option.
    const string help;                  ///< Help text for the option.

    // values is a map, containing either:
    // - a single scalar
    //   Example:
    //    runno: 14
    //
    // - a sequence of maps
    //   Example 1:
    //     gparticle:
    //       - pname: e-
    //         multiplicity: 1
    //   Example 2:
    //     verbosity:
    //       - fields: 1
    //       - particles: 2


    YAML::Node value;


    YAML::Node defaultValue;            ///< The default value of the option.
    vector <string> gvar_descs;          ///< Descriptions for each value in the sequence.
    vector <string> mandatory_keys;      ///< Keys that must be present in the option.


    /**
     * @brief Saves the option value to a YAML configuration file.
     * @param yamlConf Pointer to the output file stream.
     */
    void saveOption(std::ofstream *yamlConf) const;

    /**
     * @brief Prints the help text for the option.
     * @param detailed If true, prints detailed help; otherwise, prints a summary.
     */
    void printHelp(bool detailed) const;

    /**
     * @brief Returns detailed help text for the option.
     * @return A string containing detailed help information.
    */
    string detailedHelp() const;

    /**
     * @brief Sets the value of the scalar option based on a command line string.
     * @param v The string value to set.
     */
    void set_scalar_value(const string &v);

    /**
      * @brief Sets the value of the option based on a parsed YAML node.
      * @param v The YAML node value to set.
      */
    void set_value(const YAML::Node &v);

    /**
      * @brief Checks if all necessary values marked as `NODFLT` are set.
      * @param v The YAML node to check.
      * @return True if all necessary values are set; false otherwise.
      */
    bool does_the_option_set_all_necessary_values(YAML::Node v);

    friend class GOptions; ///< Declares `GOptions` as a friend class to access private members and functions.


};


#endif
