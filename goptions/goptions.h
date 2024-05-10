#ifndef  GOPTIONS_H
#define  GOPTIONS_H  1

#include "goption.h"
#include "gswitch.h"

//#include "goptionsConventions.h"
//#include "gutsConventions.h"


// c++
#include <string>

using std::map;


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
    vector <string> find_yaml(int argc, char *argv[]);  // finds the yaml specified by command line. Returns "na' if not found.

    // loops over all options and print web formated help
    void print_web_help();

    // instrospection
    void print_version();

public:

    // add a command line switch to the map of switches
    void addSwitch(string name, string description);

    // add a simple option to the map of options
    void addOption(string name, string description, string defaultValue, string help);

    // add a map option to the map of options
    void addOption(string name, string description, vector<string> defaultValue, string help);


    /**
     * @details Print Settings
     * \param withDefaults if true prints also the default options not assigned by the user
     */
    void print_settings(bool withDefaults);

    int getInt(string tag);       // gets the integer value 
    float getFloat(string tag);   // gets the float value 
    double getDouble(string tag); // gets the double value 
    string getString(string tag); // gets the string value 
    bool getSwitch(string tag);   // gets the bool value

    // loops over all options and print help
    void print_help();

};

// overloaded operator to add option vectors and switch maps
vector <GOption> &operator+=(vector <GOption> &original, vector <GOption> optionsToAdd);

map <string, GSwitch> &operator+=(map <string, GSwitch> &original, map <string, GSwitch> optionsToAdd);


#endif
