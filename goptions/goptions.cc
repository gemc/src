// goptions 
#include "goptions.h"

// c++
#include <iostream>

// gutilities
#include "gutilities.h"
using namespace gutilities;
using namespace std;

// constructor:
// - load user defined options, add goptions options
// - assign internal options (gdebug, gstrict)
// - parse the base jcard plus all imported jcards
// - parse the command line options
// - get our own option
GOptions::GOptions(int argc, char *argv[], vector<GOption> goptionDefinitions)
{

	// check if gdebug, gstrict, help are set on the command line
	// gdebug, gstrict needs to be the very first thing set cause it affects the construction of all objects
	for(int i=1; i<argc; i++) {
		if ( strcmp(argv[i], GSTRICTSTRING) == 0 ) {
			gstrict = true;
		} else if ( strcmp(argv[i], GDEBUGSTRING) == 0 ) {
			gdebug = true;
		} else if ( strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--h") == 0  || strcmp(argv[i], "-help") == 0  || strcmp(argv[i], "--help") == 0 ) {
			printHelp = true;
		} else if ( strcmp(argv[i], "-hweb") == 0 ) {
			printWHelp = true;
		}
	}

	cout << endl;
	if ( gdebug ) {
		cout << ARROWITEM << BOLDWHHL << "gdebug" << RSTHHR << " is set. " << endl;
	}
	if ( gstrict ) {
		cout << ARROWITEM << BOLDWHHL << "gstrict" << RSTHHR << " is set. " << endl;
	}

	// adding non switches goptionDefinitions to goptions,
	// and switches to map
	for(auto &optiond: goptionDefinitions) {
		if (  optiond.isSwitch ) {
			addSwitch(optiond.name, optiond.description);
		} else {
			goptions.push_back(optiond);
		}
	}

	// adding our options
	for(auto &ourOption: defineGOptionsOptions()) {
		goptions.push_back(ourOption);
	}

	// print help and exit if printHelp
	if ( printHelp ) {
		printOptionsHelp();
	}

	// print web formatted help and exit if printWHelp
	if ( printWHelp ) {
		printOptionsWebHelp();
	}

	// parsing command line to check if any switch is turned on
	for(int i=1; i<argc; i++) {
		string candidateSwitch = string(argv[i]);
		if ( candidateSwitch[0] == '-' ) {
			for (auto& [switchName, swiitchValue] : switches) {
				string candidateRoot = candidateSwitch.substr(1, candidateSwitch.size() - 1);

				if ( switchName ==  candidateRoot ) {
					swiitchValue.turnOn();

					if ( gdebug ) {
						cout  << GREENSQUAREITEM << "Switch option " << BOLDWHHL << candidateRoot << RSTHHR << " is set" << endl;
					}

				}
			}
		}
	}

	// finds a configuration file (jcard). Returns "na' if not found.
	jcardFilename = findBaseJCard(argc, argv);

	// parsing json can throw
	// returns all jsons objects pointed by the base and imported jcards
	try {
		vector<json> userJsonAssignments = getUserJsonsFromJCard(jcardFilename);
		parseJSONSIntoGOption(userJsonAssignments);
	}
	catch (exception& e) {
		string thisException = e.what();

		// parse error
		if (thisException.find("parse_error") != string::npos) {
			cerr << FATALERRORL << "parsing " << jcardFilename
			<< " failed. Try validating the jcard by copying its content in: " << " https://codebeautify.org/jsonvalidator" << endl;
		} else {
			cerr << FATALERRORL << "exception: " << thisException << endl;
		}

		gexit(EC__JSONPARSEERROR);
	}

	// parse command line



}


// Finds the configuration file (jcard). Returns "na' if not found.
// This also sets the verbosity
string GOptions::findBaseJCard(int argc, char *argv[])
{
	// finds jcard file as one of the argument
	// extension is .jcard
	for(int i=1; i<argc; i++) {
		string arg = argv[i];

		size_t pos = arg.find(".jcard");
		if(pos != string::npos) return arg;
	}
	return UNINITIALIZEDSTRINGQUANTITY;
}


// returns a vector of json objects of the base jcard plus all imported jcards
vector<json> GOptions::getUserJsonsFromJCard(string jcardFilename)
{
	vector<json> userJsons;

	// nothing happens if jcard wasn't found
	if (jcardFilename == UNINITIALIZEDSTRINGQUANTITY) {
		return userJsons;
	}

	// base jcard
	// removing '#' from "base" (command line) jcard
	// function is defined in gutilities
	string basePureJsonString = parseFileAndRemoveComments(jcardFilename, "#");

	if ( gdebug ) {
		cout << endl << GREENSQUAREITEM << " Parsing base jcard content: " << endl << basePureJsonString << endl;
	}

	// building json object from base jcard
	json baseJson = json::parse(basePureJsonString);

	// now imports imported jcards, if any
	for (auto& [key, value] : baseJson.items()) {
		if ( key == IMPORTJCARD ) {

			// value is a vector<string>
			for ( auto &importedJCardName: value) {
				// adding extension to set fileName
				string importJcardFileName = replaceCharInStringWithChars(importedJCardName, "\"", "") + ".jcard";

				// import json (this exit if filename isn't there)
				string importedParsedJson = parseFileAndRemoveComments(importJcardFileName);

				// add imported json to userJsons vector
				userJsons.push_back(json::parse(importedParsedJson));
			}
		}
	}

	// appending the base jcard json at the end:
	// all imports should be declared at the top of the jcard thus they come before the base settings
	userJsons.push_back(baseJson);
	return userJsons;
}


// parse all the jsons from the jcards / command line in the GOptions array
void GOptions::parseJSONSIntoGOption(vector<json> allUserJsons)
{
	// looping over all jsons
	for (auto& userJsonOption: allUserJsons) {

		// looping over all json inside each userJsonOption
		for (auto& [userJsonKey, userJsonValue] : userJsonOption.items()) {

			// skipping import directives, the imported json are already here
			if (userJsonKey == IMPORTJCARD) {
				continue;
			}

			// if the first character is "+", this is an addition
			bool isAnAddition = ( userJsonKey.front() == '+' );

			string userJsonKeyRoot = userJsonKey;

			// if it's an addition, remove first char
			if ( isAnAddition ) {
				userJsonKeyRoot = userJsonKey.substr(1, userJsonKey.size() - 1);
			}

			if ( gdebug ) {
				cout << endl << GREENSQUAREITEM << "Looking to assign Json Key " << BOLDWHHL << userJsonKey << RSTHHR << endl;
				cout << GTAB << "Content: " << userJsonValue << endl;
				cout << GTAB << "userJsonKeyRoot: " << userJsonKeyRoot  << endl;
				cout << GTAB << "Is an addition: " << isAnAddition << endl;
			}

			// GOption index, UNINITIALIZEDNUMBERQUANTITY if not found
			long userJsonOptionDefinitionIndex = findOptionIndex(userJsonKeyRoot);

			if ( gdebug ) {
				cout << GREENSQUAREITEM << "Option " << BOLDWHHL << userJsonKeyRoot << RSTHHR << " definition found." << endl;
			}

			goptions.at(userJsonOptionDefinitionIndex).assignValuesFromJson(userJsonKey, userJsonValue, isAnAddition, gdebug, gstrict);
		}
	}
}

// find GOption index from the vector<GOption>
// error if GOption is not found
long GOptions::findOptionIndex(string name) {

	for (auto it = goptions.begin(); it != goptions.end(); it++) {
		if (it->name == name) {
			return distance(goptions.begin(), it);
		}
	}

	// not found, error
	cerr << FATALERRORL << "the option " << YELLOWHHL << name << RSTHHR << " is not known to this system. " << endl;
	cerr << "Use option " << PRINTALLOPTIONS << " to print all availaible options " << endl;
	gexit(EC__NOOPTIONFOUND);

	return UNINITIALIZEDNUMBERQUANTITY;
}


vector<json> GOptions::getStructuredOptionAssignedValues(string tag) {

	// this will exit if no option is found
	long optionIndex = findOptionIndex(tag);

	return goptions[optionIndex].jOptionAssignedValues;

}


void GOptions::addSwitch(string name, string description) {
	if ( switches.find(name) == switches.end() ) {
		switches[name] = GSwitch(description);
	} else {
		if ( gstrict ) {
			cerr << FATALERRORL << "the " << YELLOWHHL << name << RSTHHR << " switch is already present." << endl;
			gexit(EC__SWITCHALREADYPRESENT);
			// non strict: warning and clear
			// the last appereance of the option is the valid one
		} else {
			cout << GWARNING " the " << YELLOWHHL << name << RSTHHR << " switch is already present." << endl;
		}
	}
}

// print only the non default settings set by users
void GOptions::printSettings(bool withDefaults)
{
	// making sure at least one option has value
	bool canPrint = false;

	for(auto& jOption: goptions) {
		if ( jOption.jOptionAssignedValues.size() ) {
			canPrint = true;
		}
	}

	// nothing to do.
	if (!canPrint) {
		cout << KGRN << " No settings defined. " << RST << endl;
		return;
	}

	if ( withDefaults ) {
		cout << endl << KGRN << " All Settings: " << RST << endl << endl;
	} else {
		cout << endl << KGRN << " Non Default User Settings: " << RST << endl << endl;
	}
	// switches
	for (auto [name, switchValue]: switches) {
		if ( withDefaults ) {
			// print all switches
			cout << KGRN << ARROWITEM << name << RST << ": " << (switchValue.getStatus() ? "on" : "off") << endl;
		} else {
			// only print the active switches
			if (switchValue.getStatus() ) {
				cout << KGRN << ARROWITEM << name << RST << ": on" << endl;
			}
		}
	}

	for(auto& jOption: goptions) {

		jOption.printOption(withDefaults);

		// non structured option, the jOptionAssignedValues has only one object, the json size is 1
		if ( jOption.jOptionAssignedValues.size() == 1 && jOption.jOptionAssignedValues.front().size() == 1 ) {
		} else {
			cout << endl;
		}
	}
	cout << endl;
}



// same as above, but look for specifically a non structured option
// exit if the tag refers to a non structured option
json GOptions::getNonStructuredOptionSingleValue(string tag) {

	// will exit if not found
	if(getStructuredOptionAssignedValues(tag).size() == 0) {
		cerr << FATALERRORL << "the tag " << tag << " is not assigned. " << endl;
		gexit(EC__OPTIONNOTASSIGNED);
	}


	json jn = getStructuredOptionAssignedValues(tag).front();

	if ( jn.begin().value().is_structured() ) {
		cerr << FATALERRORL << "the tag " << tag << " is part of the structured option " << jn << endl;
		cerr << " Use structure projection to retrieve this option (see documentation at " << GOPTIONDOCUMENTATION << ")" << endl;
		gexit(EC__OPTIONSHOULDNOTBESTRUCTURED);
	}

	return jn;
}


int GOptions::getInt(string tag) {
	json jn = getNonStructuredOptionSingleValue(tag); // will exit if not found
	return jn[tag].get<int>();
}

float GOptions::getFloat(string tag) {
	json jn = getNonStructuredOptionSingleValue(tag); // will exit if not found
	return jn[tag].get<float>();
}

double GOptions::getDouble(string tag) {
	json jn = getNonStructuredOptionSingleValue(tag); // will exit if not found
	return jn[tag].get<double>();
}

string GOptions::getString(string tag) {
	json jn = getNonStructuredOptionSingleValue(tag); // will exit if not found
	return jn[tag].get<string>();
}

bool GOptions::getSwitch(string tag) {
	if ( switches.find(tag) != switches.end()) {
		return switches[tag].getStatus();
	} else {
		cerr << FATALERRORL  << "the " << YELLOWHHL << tag << RSTHHR << " switch is not known to this system. " << endl;
		gexit(EC__NOOPTIONFOUND);
	}
	return false;
}


// options for GOption
vector<GOption> GOptions::defineGOptionsOptions()
{
	vector<GOption> goptions;

	return goptions;

}


// overloaded operator to add option vectors
vector<GOption> &operator += (vector<GOption> &original, vector<GOption> optionsToAdd) {

	for(const auto &optionToadd : optionsToAdd) {
		original.push_back(optionToadd);
	}

	return original;
}


#include <iomanip>

// print only the non default settings set by users
void GOptions::printOptionsHelp()
{

	long int helpSize = string(HELPFILLSPACE).size() + 1;
	cout.fill('.');

	cout  << KGRN << " Usage: " << RST << endl << endl;

	for (auto& s: switches) {

		string help = "-" +  s.first + RST + " " ;
		cout  << KGRN << ARROWITEM  ;
		cout << left;
		cout.width(helpSize);
		cout << help;
		cout  << s.second.getDescription()  << endl;

	}

	for(auto& jOption: goptions) {
		jOption.printOptionHelp();
	//	cout << endl;
	}

	cout << RST << endl;
	exit(EXIT_SUCCESS);
}



// print only the non default settings set by users
void GOptions::printOptionsWebHelp()
{

	long int helpSize = string(HELPFILLSPACE).size() + 1;
	cout.fill('.');

	cout  << KGRN << " Usage: " << RST << endl << endl;

	for (auto& s: switches) {

		string help = "-" +  s.first + RST + " " ;
		cout  << KGRN << ARROWITEM  ;
		cout << left;
		cout.width(helpSize);
		cout << help;
		cout  << s.second.getDescription()  << endl;

	}

	for(auto& jOption: goptions) {
		jOption.printOptionHelp();
		//	cout << endl;
	}

	cout << RST << endl;
	exit(EXIT_SUCCESS);
}
