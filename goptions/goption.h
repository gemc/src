#ifndef  GOPTION_H
#define  GOPTION_H  1

// conventions
#include "goptionsConventions.h"

// glibrary
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
 * The GSwitch contains the switch description and its status
 */
class GSwitch {
public:

	/// empty constructor, needed for the initial empty unordered_map<string, GSwitch> switches;
	GSwitch() { return ;}

	/**
	 * @brief Constructor, status is false at construction
	 * \param d switch description
	 */
	GSwitch(string d) : description(d), status(false) { return ;}

	/// @brief turn on switch
	void turnOn() { status = true;}
	/// @brief get switch status
	bool getStatus()      { return status;}
	string getDescription() { return description;}


private:
	string description;
	bool status;

};

/**
 * The class is used to:
 * * define an option
 * * assig option values from entry in a JSON file
 * * assig option values from an entry in a command line
 */
class GOption
{
public:
	/**
	 * @brief Constructor for switches.
	 * \param name the switch name
	 * \param description the switch description
	 */
	GOption(string name, string description);

	/**
	 * @details Constructor for simple option. Non groupable. Help is the goption description
	 * \param joptionDefinition is the option in JSON format.
	 * Example: { GNAME: "runno", GDESC: "run number", GDFLT: 11 }
	 */
	GOption(json joptionDefinition);

	/**
	 * @details Constructor for structured option
	 * \param name option title
	 * \param description summary description. This is used in the search.
	 * \param joptionDefinition is the option in JSON format.
	 * Example: {
	 *    {  GNAME: "runno",      GDESC: "run number",          GDFLT: 11},
	 *    {  GNAME: "nthreads", GDESC: "number of thrads", GDFLT: 4}
	 *	 }
	 * \param help the full description of the option. Multiple lines are separated by \"\n\".
	 * \param cumulative if an option belongs to a group, options can be collected by using -add-\<name\>.\n
	 */
	GOption(string name, string description, json joptionDefinition, vector<string> help = {UNINITIALIZEDSTRINGQUANTITY}, bool cumulative = false);


private:

	const string name;         // option name
	const string description;  // summary description. This is used in the search.
	bool isSwitch = false;     // if it's a switch, it will not be added to the vector<GOption> but to the map of switches

	// the json definition contains json objects like these:
	// {
	// 	[
	//    	{ GNAME: "runno",    GDESC: "run number",       GDFLT: 11},
	//    	{ GNAME: "nthreads", GDESC: "number of thrads", GDFLT: 4}
	//		]
	// }
	const json joptionDefinition;

	// help: here we can put the full description of the option.
	// multiple lines are defined by using "\n"
	vector<string> help;

	// jOptionAssignedValues is a vector containing the user assigned option values
	//
	// It is a vector of size 1 if the option is not cumulative
	//
	// Each of the jOptionAssignedValues elements contains the option
	// assigned values, validated against joptionDefinition
	//
	// if some tags are not set, they will be set to the joptionDefinition default
	// if an option is defined with default values, it will be passed to jValues
	//
	// conditions for a valid option:
	// 1. each key must match a defined tag
	// 2. if the definition does not provide a default, the option must provide one
	vector<json> jOptionAssignedValues;
	bool isDefault = false;  // true if jOptionAssignedValues is (are) the one coming from the definition


	// utilities to characterize the option
	void checkTagIsValid(string key, bool gdebug);    // check if a tag is defined. Exit if it's not
	bool isDefaultValue(string key, json userValue);  // check if userValue matches the default value

	// if an option is cumulative, options must be collected by using -add-<name>
	// jOptionAssignedValues can have multiple entries
	const bool cumulative;

	// parse user jsons options and assign jOptionAssignedValues accordingly
	void assignValuesFromJson(string userJsonKey, json userJsonValues, bool isAddition, bool gdebug, bool gstrict);
	void assignSingleValueFromSimpleJson(string userJsonKey, json userJsonValue, bool gdebug, bool gstrict);
	json assignSingleValueFromStructuredJson(string userJsonKey, string tagInJsonValues, json userJsonValue, bool gdebug, bool gstrict);
	json assignSingleValueFromCumulativeStructuredJson(string userJsonKey, string tagInJsonValues, json userJsonValue, bool gdebug, bool gstrict); // the only difference with the above is the message


	// if a user assignment is missing values, assign the default
	// if no default was given at definition, exit
	json buildDefaultToMissingValuesJson(json userAssignedValues, bool gdebug);

	// print the options different from defaults
	// if withDefaults is true also print the defaults
	void printOption(bool withDefaults);


	void printOptionHelp();

	// making goptions friend to it can access the private variables and functions
	friend class GOptions;

};


// overloaded operator to add option vectors
vector<GOption> &operator += (vector<GOption> &original, vector<GOption> optionsToAdd);


#endif