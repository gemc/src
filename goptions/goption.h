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

struct GVariable {
    string name;
    string value;
    string description;

    GVariable(string name, string val, string description) : name(name), value(val), description(description) {}

    // overloading constructors with various types of vlue
    GVariable(string name, double val, string description) : name(name), description(description) { value = std::to_string(val); }

    GVariable(string name, const char *val, string description) : name(name), value(val), description(description) {}

    GVariable(string name, int val, string description) : name(name), description(description) { value = std::to_string(val); }

    GVariable(string name, bool val, string description) : name(name), description(description) { value = val ? "true" : "false"; }
};

/**
 * The class is used to:
 * * define an option from a YAML::Node
 * *
 */
class GOption {
public:

    /**
     * @brief Constructor of simple option
     * \param GV a GVariable with the name, value and description
     * \param h  help for the option
     *
     * The value is set to the default value
     */

    // define a scalar option using a string as default value
    // scalar option default value cannot be NODFLT
    GOption(GVariable dv, string h) : name(dv.name), description(dv.description), help(h) {
        defaultValue = YAML::Load(name + ": " + dv.value);
        value = defaultValue;
    }


    /**
     * @brief Constructor of a sequence option
     * \param n  name of the option
     * \param desc  summary description of the option
     * \param dv  vector of GVariable with the name, default value and description of each element in the sequence
     * \param h  help for the option
     *
     * Note: if any of the values is NODFLT, the option is cumulative, otherwise it is not.
     */

    // define a map option using vector of strings as default value
    GOption(string n, string desc, vector <GVariable> dv, string h) : name(n), description(desc), help(h) {
        mandatory_keys.clear();
        YAML::Node nodes;
        for (auto v: dv) {
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

    // if any of the variables in defaultValue is NODFLT, the option is cumulative
    bool isCumulative = false;

    const string name;           // option name
    const string description;    // summary description. This is used in the search.
    const string help;           // help description. This is used in the help.
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
    vector <string> gvar_descs;     // description for each variable value sequence
    YAML::Node defaultValue;        // default value
    vector <string> mandatory_keys; // keys that must be present in the option

    // save option value
    void save_option(std::ofstream *yaml_conf);

    void print_help(bool detailed);

    string detailed_help();

    // sets the value of the scalar option based on the command line string
    void set_scalar_value(string v);

    // sets the value of the option based on the parsed yaml node
    void set_value(YAML::Node v);

    // make sure that all variables matked as NOFLT are set
    bool does_the_option_set_all_necessary_values(YAML::Node v);

    // making goptions friend to it can access the private variables and functions
    friend class GOptions;

};


#endif
