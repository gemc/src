// goptions
#include "goption.h"
#include "goptionsConventions.h"

// gemc
#include "gutilities.h"

// c++
#include <iostream>

using namespace std;

// add a value to the option if it is cumulative
void GOption::addValue(string v) {

    // if the option is not cumulative, and the current value is not the default value
    // then the option has already been set. exit with error
    if (!isCumulative() && !isDefault()) {
        cerr << FATALERRORL << "the " << YELLOWHHL << name << RSTHHR << " option is not cumulative and already set" << std::endl;
        gexit(EC__NONCUMULATIVEALREADYPRESENT);
    } else {
        YAML::Node value = YAML::Load(v);

        // if the value is not the default value, add it to the values vector
        if (value != defaultValue) {
            values.push_back(value);
        }
    }

}


// print option
void GOption::print_option(bool withDefaults) {
//	if (!jOptionAssignedValues.size()) {
//		return;
//	}
//
//	// non structured option, the jOptionAssignedValues has only one object, the json size is 1
//	if ( jOptionAssignedValues.size() == 1 && jOptionAssignedValues.front().size() == 1 ) {
//		json onlyOption = jOptionAssignedValues.front();
//
//		bool isDefault = (onlyOption.begin().value() == joptionDefinition[GDFLT]);
//
//		if ( withDefaults ) {
//			string isDefaultString = isDefault ? " (default)" : "";
//
//			cout << KGRN << ARROWITEM << onlyOption.begin().key() << RST << ": " << onlyOption.begin().value() << isDefaultString << endl;
//		} else {
//			if ( !isDefault) {
//				cout << KGRN << ARROWITEM << onlyOption.begin().key() << RST << ": " << onlyOption.begin().value()  << endl;
//			}
//		}
//
//		return;
//	}

    // structured option

//	cout << KGRN << ARROWITEM << name << RST << ":" << endl << endl;
//
//	for (auto& jValue: jOptionAssignedValues) {
//
//		if (cumulative) {
//			cout << TPOINTITEM ;
//			for (auto& [jValueKey, jValueValue] : jValue.items()) {
//				cout << jValueKey << ": " << jValueValue << "\t";
//			}
//			cout << endl;
//		} else {
//			for (auto& [jValueKey, jValueValue] : jValue.items()) {
//				cout << TPOINTITEM << jValueKey << ": " << jValueValue << endl;
//			}
//			//cout << endl;
//		}
//	}

}

#include <iomanip>


// print option
void GOption::print_help() {

	long int helpSize = string(HELPFILLSPACE).size() + 1;
	//string defaultValue = "";

	string helpString =  name + RST  ;

    cout << " hello " << defaultValue << endl;


//	if ( joptionDefinition.front().is_structured() ) {
//		helpString += "=<jsonvalue>" ;
//	} else {
//		helpString += "=<value>" ;
//	}
//	helpString += " ";
//
//	cout  << KGRN << ARROWITEM  ;
//	cout << left;
//	cout.width(helpSize);
//	cout << helpString;
//	cout  << description  << endl;
//
//	// structured option
//	if ( joptionDefinition.front().is_structured() ) {
//
//		// structured option
//		cout << endl;
//		for ( auto& h: help) {
//			cout << HELPFILLSPACE << h << endl;
//		}
//
//		cout << endl << HELPFILLSPACE << "<jsonvalue>:" << endl << endl;
//
//		for (auto& [definitionJsonKey, definitionJsonValue] : joptionDefinition.items()) {
//
//			cout  << HELPFILLSPACE << POINTITEM  ;
//			cout << gutilities::replaceCharInStringWithChars(definitionJsonValue[GNAME], "\"", "") << ": " ;
//			cout << gutilities::replaceCharInStringWithChars(definitionJsonValue[GDESC], "\"", "")  << endl;
//
//		}
//		cout << endl;
//	}

    cout << RST;

}
