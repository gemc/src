#ifndef  GOPTIONS_H
#define  GOPTIONS_H  1

#include "goption.h"
#include "goptionsConventions.h"
#include "gutsConventions.h"

// json parser
#include "json.hpp"
using nlohmann::json;

// c++
#include <string>
using std::string;
using std::map;
using std::vector;


/**
 * Contains STL the (private) GOption array
 */
class GOptions
{

public:

	/**
	 * @details Users Constructor
	 * \param argc number of arguments, passed from "main"
	 * \param argv argument arrays of *chars, passed from main
	 * \param goptionDefinitions vector of user options, usually returned by a defineOptions() function
	 */
	GOptions(int argc, char *argv[], vector<GOption> goptionDefinitions);

private:

	// read directly in the command line to control option debugging
	// an option cannot be used because the parsing is part of the debug
	// activate debug logging
	bool gdebug = false;

	// read directly in the command line to control option debugging
	// an option cannot be used because the parsing is part of the debug
	// activate exit on:
	// - duplicate options
	bool gstrict = false;

	// print option help and exit if set to true
	// with any of these flags: -h --h -help --help
	bool printHelp = false;

	// print option web formatted help and exit if set to true
	// with any of these flags: -hweb
	bool printWHelp = false;


	// GOption array
	vector<GOption> goptions;

	// Switches map
	map<string, GSwitch> switches;

	// jcards parsing utilities
	string findBaseJCard(int argc, char *argv[]);  // finds a configuration file (jcard). Returns "na' if not found.

	vector<json> getUserJsonsFromJCard(string jcardFilename);    // returns all jsons objects pointed by the base and imported jcards

	void parseJSONSIntoGOption(vector<json> allUserJsons);       // parse all the jsons from the jcards / command line in the GOptions array

	// search utilities
	long findOptionIndex(string name);  // find goption from the array. return jOptions array index or UNINITIALIZEDNUMBERQUANTITY if not found

	// same as above, but look for specifically a non structured option
	// exit if the tag refers to a non structured option
	json getNonStructuredOptionSingleValue(string tag);

	// options for GOptions
	vector<GOption>  defineGOptionsOptions();

	// add a switch to the map of switches
	void addSwitch(string name, string description);

	// loops over all options and print help
	void printOptionsHelp();

	// loops over all options and print web formattd help
	void printOptionsWebHelp();

    // introspection
    void print_version();

public:

	/**
	 * @details Print Settings
	 * \param withDefaults if true prints the options not assigned by the user
	 */
	void printSettings(bool withDefaults);

	int getInt(string tag);       // gets the integer value associated with non structured option \"tag\"
	float getFloat(string tag);   // gets the float value associated with non structured option \"tag\"
	double getDouble(string tag); // gets the double value associated with non structured option \"tag\"
	string getString(string tag); // gets the string value associated with non structured option \"tag\"
	bool getSwitch(string tag);   // gets the bool value associated with switch \"tag\"

	string jcardFilename;         // jcard with path, specified by the command line

	/**
	 * @brief Get the (structured) option json values corresponding to a tag
	 * \param tag the structured option json tag
	 * Used to project options onto structures (see structuredExample)
	 */
	vector<json> getStructuredOptionAssignedValues(string tag);


};

// overloaded operator to add option vectors
vector<GOption> &operator += (vector<GOption> &original, vector<GOption> optionsToAdd);


#endif
