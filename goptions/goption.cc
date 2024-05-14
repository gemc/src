// goptions
#include "goption.h"
#include "goptionsConventions.h"

// gemc
#include "gutilities.h"

// c++
#include <iostream>

using namespace std;

// sets the value to the scalar option
void GOption::set_value(string v) {
    string value_to_set = gutilities::replaceCharInStringWithChars(v, ",", "");

    auto scalar = values.front();
    string key = scalar.begin()->first.as<string>();
    scalar[key] = value_to_set;
}


// sets the value of the option based on the parsed yaml node
void GOption::set_value(YAML::Node v) {
    // a cumulative option will have one of the entries as NODFLT
    // in this case values is empty, just need to push back the value here
    if (values.size() == 0) {
        // version is a special option
        if (name != GVERSION_STRING) {
            values.push_back(v);
        }
    } else {
        // if the option is not cumulative, the value
        // is set to the default value.
        // we only need to replace the items in the sequence that are not the default value
        auto first_option_map = values.front().begin()->second;
        for (auto element: v) {
            for (YAML::const_iterator it = element.begin(); it != element.end(); ++it) {
                for (auto items: first_option_map) {
                    for (YAML::const_iterator itv = items.begin(); itv != items.end(); ++itv) {
                        string first_key = itv->first.as<string>();
                        string second_key = it->first.as<string>();
                        if (first_key == second_key) {
                            items[itv->first] = it->second;

                        }
                    }
                }
            }
        }
    }
}


// print the option
void GOption::save_option(ofstream &yaml_conf) {

    for (auto &value: values) {

        if (value.size() == 1) {
            yaml_conf << value << endl;
        } else {
            yaml_conf << name << ":" << endl;

            switch (value.Type()) {
                case YAML::NodeType::Sequence:
                    for (auto element: value) {
                        for (YAML::const_iterator it = element.begin(); it != element.end(); ++it) {
                            if (it == element.begin()) {
                                yaml_conf << "  - " << it->first << " " << it->second << endl;
                            } else {
                                yaml_conf << "    " << it->first << " " << it->second << endl;
                            }
                        }
                    }
                    break;
                default:
                    break;
            }
        }
    }
}


// print option
void GOption::print_help(bool detailed) {
    if (name == GVERSION_STRING) {
        return;
    }

    long int fill_width = string(HELPFILLSPACE).size() + 1;
    cout.fill('.');

    string helpString = "-" + name + RST;

    bool is_sequence = defaultValue.begin()->second.IsSequence();

    if (is_sequence) {
        helpString += "=<sequence>";
    } else {
        helpString += "=<value>";
    }
    helpString += " ";

    cout << KGRN << " " << left;
    cout.width(fill_width);


    if (detailed) {
        cout << helpString << ": " << description << endl;
        cout << endl;
        cout << detailed_help() << endl;
    } else {
        cout << helpString << ": " << description << endl;
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
