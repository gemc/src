#ifndef  GOPTIONS_H
#define  GOPTIONS_H  1

#include "goption.h"
#include "gswitch.h"

// c++
#include <string>
#include <fstream>
#include <iostream>


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
    GOptions(int argc, char *argv[], const GOptions &user_defined_options);

    // define and add a command line switch to the map of switches
    void defineSwitch(const std::string &name, const std::string &description);

    // add a simple option to the map of options
    void defineOption(const GVariable &gvar, const std::string &help);

    // add a map option to the map of options
    void defineOption(const std::string &name, const std::string &description, const std::vector <GVariable> &gvars, const std::string &help);

    // option getters for scalar options
    int getScalarInt(const std::string &tag) const;

    float getScalarFloat(const std::string &tag) const;

    double getScalarDouble(const std::string &tag) const;

    std::string getScalarString(const std::string &tag) const;

    bool getSwitch(const std::string &tag) const;

    // Returns the YAML::Node of the option with the tag
    inline const YAML::Node getOptionNode(const std::string& tag) const {
        // If the option does not exist, exit with error
        if (!doesOptionExist(tag)) {
            std::cerr << "Option " << tag << " does not exist. Exiting." << std::endl;
            gexit(EC__NOOPTIONFOUND);
        }
        return getOptionIterator(tag)->value.begin()->second;
    }

    YAML::Node getOptionMapInNode(string option_name, string map_key);

    int getVerbosityFor(const std::string& tag) const;

    // returns the goptions array
    const std::vector<GOption>& getOptions() const { return goptions; }

    // return the switches map
    const std::map<std::string, GSwitch>& getSwitches() const { return switches; }

    // adds a vector of options to the current options
    inline void addGOptions(const GOptions& goptions_to_add) {
        for (auto gopt: goptions_to_add.getOptions()) {
            goptions.push_back(gopt);
        }
        for (auto sw: goptions_to_add.getSwitches()) {
            switches.insert(sw);
        }
    }

    template<typename T>
    T get_variable_in_option(const YAML::Node &node, const string &variable_name, const T &default_value);

    vector <string> get_yaml_files() { return yaml_files; }

private:

    // GOption array
    vector <GOption> goptions;

    // Switches map
    map <string, GSwitch> switches;

    // jcards parsing utilities
    vector <string> find_yamls(int argc, char *argv[]);  // finds the yaml specified by command line. Returns "na' if not found.

    // parse the yaml file
    void set_options_values_from_yaml_file(string yamls);

    // parse a command linegetSwitch
    void set_option_values_from_command_line_argument(string option_name, string possible_yaml_node);

    // Checks if the option exists
    bool doesOptionExist(const std::string& tag) const;

    // returns vector<GOption> map iterator that has the option name
    std::vector<GOption>::iterator getOptionIterator(const std::string& name);
    std::vector<GOption>::const_iterator getOptionIterator(const std::string& name) const;

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
    std::ofstream *yaml_conf;

    // introspection.
    void print_version();

    // save yaml file locations
    vector <string> yaml_files;

};

// overloaded operator to add option vectors and switch maps
GOptions &operator+=(GOptions &original, GOptions optionsToAdd);

#endif
