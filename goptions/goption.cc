// goptions
#include "goption.h"
#include "goptionsConventions.h"

// gemc
#include "gutilities.h"

// c++
#include <iostream>

using namespace std;

// add a value to the option if it is cumulative
void GOption::set_value(string v) {

//    // if the option is not cumulative, and the current value is not the default value
//    // then the option has already been set. exit with error
//    if (!isCumulative() && !isDefault()) {
//        cerr << FATALERRORL << "the " << YELLOWHHL << name << RSTHHR << " option is not cumulative and already set" << std::endl;
//        gexit(EC__DEFINED_OPTION_ALREADY_PRESENT);
//    } else {
//        YAML::Node value = YAML::Load(v);
//
//        // if the value is not the default value, add it to the values vector
//        if (value != defaultValue) {
//            values.push_back(value);
//        }
//    }

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



// print option
void GOption::print_help(bool detailed) {

    long int fill_width = string(HELPFILLSPACE).size() + 1;
    cout.fill('.');

    string helpString = "-" + name + RST;
    if (name[0] == '+') {
        helpString = name + RST;
    }
    bool is_sequence = defaultValue.begin()->second.IsSequence();

    if (is_sequence) {
        helpString += "=<sequence>";
    } else {
        helpString += "=<value>";
    }
    helpString += " ";

    cout << KGRN << " " << left;
    cout.width(fill_width);

    string cumulative = "";
    if (name[0] == '+') {
        cumulative = " (cumulative)";
    }

    if (detailed) {
        cout << helpString << ": " << description << cumulative << endl;
        cout << endl;
        cout << detailed_help() << endl;
    } else {
        cout << helpString << ": " << description << cumulative << endl;
    }

}

string GOption::detailed_help() {
    string newHelp = "";


    YAML::Node yvalues = defaultValue.begin()->second;
    if (yvalues.IsSequence()) {
        newHelp += "\n";

        for (auto i = 0; i < yvalues.size(); i++) {
            YAML::Node this_node = yvalues[i];

            for (YAML::const_iterator it = this_node.begin(); it != this_node.end(); ++it) {
                cout << TGREENPOINTITEM << KGRN << it->first.as<string>() << RST
                << ": " << gvar_descs[i] << ". Default value: " << it->second.as<string>() << endl;
            }

        }
    }
    newHelp += "\n";

    vector <string> help_lines = gutilities::getStringVectorFromStringWithDelimiter(help, "\n");

    for (auto line: help_lines) {
        newHelp += GTAB + line + "\n";
    }
    return newHelp;

}
