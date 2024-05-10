#ifndef  GOPTION_H
#define  GOPTION_H  1

// goption
#include "goptionsConventions.h"

// gemc
#include "gutsConventions.h"

// c++
using std::string;
using std::vector;

// yaml-cpp
#include "yaml-cpp/yaml.h"


/**
 * The class is used to:
 * * define an option from a YAML::Node
 * *
 */
class GOption {
public:

    /**
     * @brief Constructor
     * \param n option name
     * \param d option description
     * \param dv default value
     * \param h help description
     *
     * The value is set to the default value
     */

    // define a simple option using a string as default value
    GOption(string n, string d, string dv, string h) : name(n), description(d), help(h) {
        defaultValue = YAML::Load(dv);
        values.clear();
        values.push_back(defaultValue);
    }

    // define a map option using vector of strings as default value
    GOption(string n, string d, vector<string> dv, string h) : name(n), description(d), help(h) {
        YAML::Node node;
        for (auto v: dv) {
            node.push_back(YAML::Load(v));
        }
        node["key"] = "value";
        defaultValue = node;
    }

    // add a value to the option if it is cumulative
    void addValue(string v);

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
    YAML::Node defaultValue;     // default value

    // print the options different from defaults
    // if withDefaults is true also print the defaults
    void print_option(bool withDefaults);

    void print_help();

    // making goptions friend to it can access the private variables and functions
    friend class GOptions;

};




#endif
