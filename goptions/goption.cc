// goptions
#include "goptions.h"

// gutilities for the conventions and gexit
#include "gutilities.h"

// c++
#include <iostream>
using namespace std;

// constructor for switch option
// if an option is defined with default values, it will be passed to jUserValues
// users reset default values in the jcard or command lines
GOption::GOption(string name, string d):
name(name),
description(d),
isSwitch(true),
cumulative(false)
{
	return;
}


// constructor for simple option
// if an option is defined with default values, it will be passed to jUserValues
// users reset default values in the jcard or command lines
GOption::GOption(json j):
name(j[GNAME]),
description(j[GDESC]),
joptionDefinition(j),
cumulative{false}
{
	// assigning defaults values
	json jValue;
	for (auto& h: joptionDefinition[GDESC]) {
		help.push_back(h);
	}

	// by constructon the keys will always be there
	auto defaultValue = joptionDefinition[GDFLT];

	// if the option can be assigned the default value, do it here
	if ( defaultValue != NODFLT ) {
		string jKey  = joptionDefinition[GNAME];
		jValue[jKey] = defaultValue;
		jOptionAssignedValues.push_back(jValue);
		isDefault = true;
	}

	return;
}

// constructor for structured option
// if an option is defined with default values, it will be passed to jUserValues
// users reset default values in the jcard or command lines
GOption::GOption(string n, string d, json j, vector<string> h, bool m):
name(n),
description(d),
joptionDefinition(j),
help(h),
cumulative(m)
{
	// don't do anything if any tag has GDFLT = NODFLT
	for (auto& [definitionJsonKey, definitionJsonValue] : joptionDefinition.items()) {

		if ( definitionJsonValue[GDFLT] == NODFLT ) {
			// nothing to do, user will have to define this option
			return;
		}
	}

	// all tags in this option have default values
	// assigning structured option default values
	json newUserValue;

	for (auto& [definitionJsonKey, definitionJsonValue] : joptionDefinition.items()) {
		string optionKey        = definitionJsonValue[GNAME];
		newUserValue[optionKey] = definitionJsonValue[GDFLT];
		isDefault               = true;
	}

	jOptionAssignedValues.push_back(newUserValue);
}




// parse user jsons options and assign jValues accordingly
// this exits if:
//
// - a tag is not defined
// - add- was not used for a cumulative option
// - add- was used for a non cumulative option
// - user did not provide a value that must be set (options w/o default values)
//
// These options come ordered
// if a groupable option didn't have the add directive, jValues is cleared
void GOption::assignValuesFromJson(string userJsonKey, json userJsonValues, bool isAddition, bool gdebug, bool gstrict)
{
	// if "+" was not found but option is cumulative, it's a mistake.
	if ( !isAddition && cumulative ) {
		cerr << FATALERRORL << "the " << YELLOWHHL << userJsonKey << RSTHHR << " tag is cumulative. Mandatory \"+\" directive was not given. " << endl;
		gexit(EC__NOADDFORCUMULATIVE);
	}

	// if "+" was found but option is not cumulative, it's a mistake.
	if ( isAddition && !cumulative ) {
		cerr << FATALERRORL << "the " << YELLOWHHL << userJsonKey << RSTHHR << " tag is non cumulative but \"+\" was given. " << endl;
		gexit(EC__ADDFORNONCUMULATIVE);
	}

	// simple option
	if ( ! userJsonValues.is_structured() ) {

		// if userJsonValue is default, nothing to do
		if ( isDefaultValue(userJsonKey, userJsonValues) ) {
			if ( gdebug ) {
				cout << TGREENPOINTITEM << "Simple option " << YELLOWHHL << userJsonKey << RSTHHR << " is assigned the default value. " << userJsonValues << ". Nothing to do." << endl;
			}
			return;
		}

		assignSingleValueFromSimpleJson(userJsonKey, userJsonValues, gdebug, gstrict);

	} else if ( ! userJsonValues.is_array() ) {

		// non cumulative structured option
		json newUserValue;

		// non cumulative structure option, looping over tags
		// looping over all user jsons
		for (auto& [userJsonKeyInValues, userJsonValueInValue] : userJsonValues.items()) {
			newUserValue[userJsonKeyInValues] = assignSingleValueFromStructuredJson(userJsonKey, userJsonKeyInValues, userJsonValueInValue, gdebug, gstrict);
		}

		// at this point all json keys are valid, and the user json keys are assigned properly
		// we need to assign default values for all the keys the user didn't set
		// if some of the unset values option must provide a default, this routine will exit with error
		json unassigned = buildDefaultToMissingValuesJson(userJsonValues, gdebug);

		for (auto& [unassignedKey, unassignedValue] : unassigned.items())  {
			newUserValue[unassignedKey] = unassignedValue;
		}

		// no unset key found at this point
		// adding the newUserValue
		// valid, non default, assigning it
		jOptionAssignedValues.clear();
		jOptionAssignedValues.push_back(newUserValue);
		isDefault = false;

	} else {

		// cumulative structured option
		vector<json> newUserValues;

		for (auto& userJsonValueItem: userJsonValues) {

			json singleNewUserValue;

			for (auto& [userJsonKeyInValues, userJsonValueInValue] : userJsonValueItem.items()) {
				singleNewUserValue[userJsonKeyInValues] = assignSingleValueFromCumulativeStructuredJson(userJsonKey, userJsonKeyInValues, userJsonValueInValue, gdebug, gstrict);

				// at this point all json keys are valid, and the user json keys are assigned properly
				// we need to assign default values for all the keys the user didn't set
				// if some of the unset values option must provide a default, this routine will exit with error
				json unassigned = buildDefaultToMissingValuesJson(userJsonValueItem, gdebug);

				for (auto& [unassignedKey, unassignedValue] : unassigned.items())  {
					singleNewUserValue[unassignedKey] = unassignedValue;
				}


			}
			newUserValues.push_back(singleNewUserValue);
		}

		// no unset key found at this point
		// adding the newUserValue
		// valid, non default, assigning it
		jOptionAssignedValues.clear();

		for ( auto& newUserValue: newUserValues) {
			jOptionAssignedValues.push_back(newUserValue);
		}
		isDefault = false;


	}

}


json GOption::buildDefaultToMissingValuesJson(json userAssignedValues, bool gdebug) {

	json unassignedJson;

	// looking for unset keys in the option definition
	for (auto& [definitionJsonKey, definitionJsonValue] : joptionDefinition.items())  {

		// looping over tags inside option definition
		for ( auto& jsonTagValue : definitionJsonValue.items() )  {

			// main structure option GOption tag
			if ( jsonTagValue.key() == GNAME ) {

				bool thisTagWasFoundAndAssigned = false;
				string tagToCheck = jsonTagValue.value() ;

				if ( gdebug ) {
					cout << TGREENPOINTITEM << "Checking if user key " << YELLOWHHL << tagToCheck << RSTHHR << " is assigned " << endl;
				}

				// structure option, looping over tags
				// looping over all user jsons
				for (auto& [userJsonKeyInValues, userJsonValueInValue] : userAssignedValues.items()) {
					if ( tagToCheck == userJsonKeyInValues ) {
						thisTagWasFoundAndAssigned = true;
						if ( gdebug ) {
							cout << TTGREENPOINTITEM << "User key " << YELLOWHHL << tagToCheck << RSTHHR << " is assigned " << endl;
						}
					}
				}

				// tag value not assigned.
				// Assign the default value if it's defined.
				// Exiting if it was mandatory.
				if( !thisTagWasFoundAndAssigned ) {

					if (definitionJsonValue[GDFLT] == NODFLT) {
						cerr << FATALERRORL << tagToCheck <<  " in " << definitionJsonValue << " is marked mandatory but it's not set." << endl;
						gexit(EC__MANDATORYOPTIONNOTFOUND);
					}

					// assigning its default value
					unassignedJson[tagToCheck] = definitionJsonValue[GDFLT];
					if ( gdebug ) {
						cout << TTGREENPOINTITEM << "User key " << YELLOWHHL << tagToCheck << RSTHHR ;
						cout << " is not assigned. Setting it to " << HHL << definitionJsonValue[GDFLT] << HHR << endl;
					}
				}
			}
		}
	}
	return unassignedJson;
}


void GOption::assignSingleValueFromSimpleJson(string userJsonKey, json userJsonValue, bool gdebug, bool gstrict) {

	// userJsons is simple
	if ( gdebug ) {
		cout << TGREENPOINTITEM << "Assigning simple option " << BOLDWHHL << userJsonKey << RSTHHR << " with value " << HHL << userJsonValue << HHR << endl;
	}

	checkTagIsValid(userJsonKey, gdebug);

	// overwriting simple options is allowed if gstrict is not set.
	// a warning is given
	if ( jOptionAssignedValues.size() && !isDefault ) {
		// strict: error
		if ( gstrict ) {
			cout << FATALERRORL << "the " << YELLOWHHL << userJsonKey << RSTHHR << " tag is non cumulative, is already present, and it's not the default. " << endl;
			gexit(EC__NONCUMULATIVEALREADYPRESENT);
			// non strict: warning and clear
			// the last appereance of the option is the valid one
		} else {
			cout << GWARNING " the " << YELLOWHHL << userJsonKey << RSTHHR << " tag is non cumulative and is already present, and it's not the default." << endl;
		}
	}

	json newUserValue;
	auto assignedValue = userJsonValue.items().begin().value();

	// tag is valid, assigning it
	newUserValue[userJsonKey] = assignedValue;
	if ( gdebug ) {
		cout << TGREENPOINTITEM << " Json Option " << GREENHHL << userJsonKey << RSTHHR << " set with value: " << HHL << assignedValue << HHR << endl;
	}

	// valid, non default assigning it
	jOptionAssignedValues.clear();
	jOptionAssignedValues.push_back(newUserValue);
	isDefault = false;
}


json GOption::assignSingleValueFromStructuredJson(string userJsonKey, string tagInJsonValues, json userJsonValue, bool gdebug, bool gstrict) {

	// userJsons is structured
	if ( gdebug ) {
		cout << TGREENPOINTITEM << "Assigning single structured option " << BOLDWHHL << userJsonKey << RSTHHR << endl;
	}

	checkTagIsValid(tagInJsonValues, gdebug);

	// tag is valid, returning it
	if ( gdebug ) {
		cout << TTPOINTITEM << "Assigning single user key " << YELLOWHHL << tagInJsonValues << RSTHHR << " with value: " << HHL << userJsonValue << HHR << endl;
	}

	return userJsonValue;
}

json GOption::assignSingleValueFromCumulativeStructuredJson(string userJsonKey, string tagInJsonValues, json userJsonValue, bool gdebug, bool gstrict) {

	checkTagIsValid(tagInJsonValues, gdebug);

	// tag is valid, returning it
	if ( gdebug ) {
		cout << TTPOINTITEM << BOLDWHHL << userJsonKey << RSTHHR << ": assigning single user key " << YELLOWHHL << tagInJsonValues << RSTHHR << " with value: " << HHL << userJsonValue << HHR << endl;
	}

	return userJsonValue;
}



// check if a userValue in this option matches the default value
bool GOption::isDefaultValue(string key, json userValue) {

	// false if there is no default
	if ( ! isDefault ) return false;

	bool isUserDefault = false;

	// simple option
	if ( ! userValue.is_structured() ) {
		auto defaultValue =  jOptionAssignedValues.front().items().begin().value();

		// looking for unset keys in the option definition
		// Example: { GNAME: "runno", GDESC: "run number", GDFLT: 11 }
		for (auto& [definitionJsonKey, definitionJsonValue] : joptionDefinition.items())  {

			// user assigned an option definition
			if ( key == definitionJsonValue ) {
				if ( userValue == defaultValue ) {
					isUserDefault = true;
				}
			}
		}
	} else {
		// structured option. If all values are user defaults, then set the switch to true
		bool allDefault = true;
		for (auto& [definitionJsonKey, definitionJsonValue] : joptionDefinition.items())  {

		for ( auto& jsonTagValue : definitionJsonValue.items() )  {
			if ( jsonTagValue.key() == GNAME && key == jsonTagValue.value() ) {
//				if ( gdebug ) {
//					cout << TTPOINTITEM << "key matches " << jsonTagValue.value() << endl;
//				}
//				isDefined = true;
			}
		}
		}
		isUserDefault = allDefault;

	}


	return isUserDefault;
}


// checking if the key is the json object is defined
void GOption::checkTagIsValid(string key, bool gdebug) {
	
	bool isDefined = false;

	// joptionDefinition is a json
	// Non Structured Example: { GNAME: "runno",  GDESC: "run number", GDFLT: 11} }
	//
	// Structured Example: {
	//    { GNAME: "runno",    GDESC: "run number",       GDFLT: 11},
	//    { GNAME: "nthreads", GDESC: "number of thrads", GDFLT: 4}
	// }
	for (auto& [definitionJsonKey, definitionJsonValue] : joptionDefinition.items()) {

		// non structured option first
		// matching key to GNAME definition
		if ( !definitionJsonValue.is_structured() ) {

			if ( gdebug ) {
				cout << TTPOINTITEM << "Checking user key " << key << " against definition item tag " << definitionJsonValue << endl;
			}

			if ( definitionJsonKey == GNAME) {

				if (key == definitionJsonValue) {
					if ( gdebug ) {
						cout << TTPOINTITEM << "key matches " << definitionJsonValue << endl;
					}
					isDefined = true;
				}
			}

			// structured option
			// when key is GNAME, matching key to definition value
		} else {

			for ( auto& jsonTagValue : definitionJsonValue.items() )  {
				if ( jsonTagValue.key() == GNAME && key == jsonTagValue.value() ) {
					if ( gdebug ) {
						cout << TTPOINTITEM << "key matches " << jsonTagValue.value() << endl;
					}
					isDefined = true;
				}
			}

		}
	}

	if ( !isDefined )  {
		cerr << FATALERRORL << "the " << YELLOWHHL << key << RSTHHR << " tag is not known to this system. " << endl;
		gexit(EC__NOOPTIONFOUND);
	}

}


// print option
void GOption::printOption(bool withDefaults)
{
	if (!jOptionAssignedValues.size()) {
		return;
	}

	// non structured option, the jOptionAssignedValues has only one object, the json size is 1
	if ( jOptionAssignedValues.size() == 1 && jOptionAssignedValues.front().size() == 1 ) {
		json onlyOption = jOptionAssignedValues.front();

		bool isDefault = (onlyOption.begin().value() == joptionDefinition[GDFLT]);

		if ( withDefaults ) {
			string isDefaultString = isDefault ? " (default)" : "";

			cout << KGRN << ARROWITEM << onlyOption.begin().key() << RST << ": " << onlyOption.begin().value() << isDefaultString << endl;
		} else {
			if ( !isDefault) {
				cout << KGRN << ARROWITEM << onlyOption.begin().key() << RST << ": " << onlyOption.begin().value()  << endl;
			}
		}

		return;
	}

	// structured option

	cout << KGRN << ARROWITEM << name << RST << ":" << endl << endl;

	for (auto& jValue: jOptionAssignedValues) {
		
		if (cumulative) {
			cout << TPOINTITEM ;
			for (auto& [jValueKey, jValueValue] : jValue.items()) {
				cout << jValueKey << ": " << jValueValue << "\t";
			}
			cout << endl;
		} else {
			for (auto& [jValueKey, jValueValue] : jValue.items()) {
				cout << TPOINTITEM << jValueKey << ": " << jValueValue << endl;
			}
			//cout << endl;
		}
	}

}

#include <iomanip>

// print option
void GOption::printOptionHelp()
{

	long int helpSize = string(HELPFILLSPACE).size() + 1;
	string defaultValue = "";

	string helpString = "-" +  name + RST  ;
	if ( cumulative ) {
		helpString = "+" +  name + RST  ;
	}

	if ( joptionDefinition.front().is_structured() ) {
		helpString += "=<jsonvalue>" ;
	} else {
		helpString += "=<value>" ;
	}
	helpString += " ";
	
	cout  << KGRN << ARROWITEM  ;
	cout << left;
	cout.width(helpSize);
	cout << helpString;
	cout  << description  << endl;

	// structured option
	if ( joptionDefinition.front().is_structured() ) {

		// structured option
		cout << endl;
		for ( auto& h: help) {
			cout << HELPFILLSPACE << h << endl;
		}
		
		cout << endl << HELPFILLSPACE << "<jsonvalue>:" << endl << endl;

		for (auto& [definitionJsonKey, definitionJsonValue] : joptionDefinition.items()) {

			cout  << HELPFILLSPACE << POINTITEM  ;
			cout << gutilities::replaceCharInStringWithChars(definitionJsonValue[GNAME], "\"", "") << ": " ;
			cout << gutilities::replaceCharInStringWithChars(definitionJsonValue[GDESC], "\"", "")  << endl;
			
		}
		cout << endl;
	}

	cout << RST ;

}
