#ifndef  GOPTIONS_H
#define  GOPTIONS_H  1

#include "goption.h"
#include "gswitch.h"


// c++
#include <string>
#include <fstream>


/**
 * Contains STL the (private) GOption array
 */
class GOptions {

public:

    // empty constructor, to be used to define the GOptions user_defined_options passed in the below constructor
    GOptions() { return; }

    /**
     * @details Library based constructor: each library can define its own options,
     * \param argc number of arguments, passed from "main"
     * \param argv argument arrays of *chars, passed from main
     * \param user_defined_options vector of user options, usually returned by a defineOptions() function
     */
    GOptions(int argc, char *argv[], GOptions user_defined_options);

private:

    // GOption array
    vector <GOption> goptions;

    // Switches map
    map <string, GSwitch> switches;

    // jcards parsing utilities
    vector <string> find_yamls(int argc, char *argv[]);  // finds the yaml specified by command line. Returns "na' if not found.

    // parse the yaml file
    void set_options_values_from_yaml_file(string yamls);

    // parse a command line
    void set_option_values_from_command_line_argument(string option_name, string possible_yaml_node);

    // checks if the option exists
    bool does_option_exist(string tag);

    // returns vector<GOption> map iterator for option name
    vector<GOption>::iterator get_option_iterator(string name);

    // search
    vector <GOption> search_for_string(string tag); // searches for a string option

    // print single option or switch
    void print_option_or_switch_help(string tag);

    // loops over all options and print help
    void print_help();

    // print web formated help
    void print_web_help();

    // Save All User Options In a Yaml File
    void save_options();

    // sets the executable name at construction
    string executableName;

    // yaml saved configuration filename
    std::ofstream yaml_conf;

    // introspection.
    void print_version();

public:

    // add a command line switch to the map of switches
    void defineSwitch(string name, string description);

    // add a simple option to the map of options
    void defineOption(GVariable gvar, string help);

    // add a map option to the map of options
    void defineOption(string name, string description, vector <GVariable> gvars, string help);


    // option getters for scalar options
    int getInt(string tag);

    float getFloat(string tag);

    double getDouble(string tag);

    string getString(string tag);

    bool getSwitch(string tag);
};

// overloaded operator to add option vectors and switch maps
vector <GOption> &operator+=(vector <GOption> &original, vector <GOption> optionsToAdd);

map <string, GSwitch> &operator+=(map <string, GSwitch> &original, map <string, GSwitch> optionsToAdd);


#endif
