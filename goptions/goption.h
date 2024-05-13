#ifndef  GOPTION_H
#define  GOPTION_H  1

// goption
#include "goptionsConventions.h"

// gemc
#include "gutsConventions.h"

// c++
#include <map>
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
    GVariable(string name, double val, string description) : name(name), description(description) {value = std::to_string(val);}
    GVariable(string name, const char *val, string description) : name(name), value(val), description(description) {}
    GVariable(string name, int val, string description) : name(name), description(description) {value = std::to_string(val);}
    GVariable(string name, bool val, string description) : name(name), description(description) {value = val ? "true" : "false";}
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

    // define a simple option using a string as default value
    GOption(GVariable dv, string h) : name(dv.name), description(dv.description), help(h) {
        defaultValue = YAML::Load(name + ": " + dv.value);
        values.clear();
        values.push_back(defaultValue);
    }


    /**
     * @brief Constructor of a sequence option
     * \param n  name of the option
     * \param desc  summary description of the option
     * \param dv  vector of GVariable with the name, value and description of each element in the sequence
     * \param h  help for the option
     *
     * The value is set to the default value
     */

    // define a map option using vector of strings as default value
    GOption(string n, string desc, vector<GVariable> dv, string h) : name(n), description(desc), help(h) {
        YAML::Node nodes;
        for (auto v: dv) {
            YAML::Node this_node = YAML::Load(v.name + ": " + v.value);
            gvar_descs.push_back(v.description);
            nodes.push_back(this_node);
        }
        defaultValue[n] = nodes;
    }



private:

    // if the name start with '+' the option is cumulative
    inline bool isCumulative() { return name[0] == '+'; }

    // if there is only one value and it is the default value
    bool isDefault() {
        return values.size() == 1 && values[0] == defaultValue;
    }

    const string name;           // option name
    const string description;    // summary description. This is used in the search.
    const string help;           // help description. This is used in the help.
    vector<YAML::Node> values;   // option values. If the option is not cumulative, the vector will have only one element
    vector<string> gvar_descs;   // description for each of the above values
    YAML::Node defaultValue;     // default value

    // print the options different from defaults
    // if withDefaults is true also print the defaults
    void print_option(bool withDefaults);

    void print_help(bool detailed);

    string detailed_help();

    // set the value of the option
    void set_value(string v);


    // making goptions friend to it can access the private variables and functions
    friend class GOptions;

};




#endif
