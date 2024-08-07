// goptions
#include "goption.h"
#include "goptionsConventions.h"

// gemc
#include "gutilities.h"

// c++
#include <iostream>

using namespace std;

// sets the value to the scalar option
void GOption::set_scalar_value(const string &v) {

    // return if v is empty
    if (v == "") return;

    string value_to_set = gutilities::replaceCharInStringWithChars(v, ",", "");

    string key = value.begin()->first.as<string>();
    value[key] = value_to_set;
}


// sets the value of the option based on the parsed yaml node
void GOption::set_value(const YAML::Node &v) {

    // if the option is cumulative,
    if (isCumulative) {
        // sequence of maps
        for (const auto &element: v) {
            if (!does_the_option_set_all_necessary_values(element)) {
                cerr << FATALERRORL << "Trying to set " << YELLOWHHL << name << RSTHHR << " but missing mandatory values." << endl;
                cerr << "        Use the option: " << YELLOWHHL << " help " << name << " " << RSTHHR << " for details." << endl << endl;
                exit(EC__MANDATORY_NOT_FILLED);
            }
        }


        value[name] = v;

        // Copy keys from the defaultValue map to the value map if they do not exist
        auto default_value_node = defaultValue.begin()->second;

        for (const auto &map_element_in_default_value: default_value_node) {
            for (YAML::const_iterator default_value_iterator = map_element_in_default_value.begin();
                 default_value_iterator != map_element_in_default_value.end(); ++default_value_iterator) {

                string default_key = default_value_iterator->first.as<string>();
                auto default_value = default_value_iterator->second;


                for (auto map_element_in_value: value[name]) {
                    bool key_found = false;
                    // checking if the key is already in the value
                    for (YAML::const_iterator value_iterator = map_element_in_value.begin();
                         value_iterator != map_element_in_value.end(); ++value_iterator) {
                        string value_key = value_iterator->first.as<string>();

                        if (default_key == value_key) {
                            key_found = true;
                            break;
                        }
                    }
                    if (!key_found) {
                        map_element_in_value[default_key] = default_value;
                    }
                }
            }
        }

    } else {

        // looping over the sequence of maps in v
        // notice: the non cumulative node is already copied from the default value
        // here we are just updating the values to the desired value
        for (auto map_element_in_desired_value: v) {

            for (YAML::const_iterator desired_value_iterator = map_element_in_desired_value.begin();
                 desired_value_iterator != map_element_in_desired_value.end(); ++desired_value_iterator) {

                for (auto existing_map: value[name]) {
                    for (YAML::const_iterator existing_map_iterator = existing_map.begin();
                         existing_map_iterator != existing_map.end(); ++existing_map_iterator) {

                        string first_key = existing_map_iterator->first.as<string>();
                        string second_key = desired_value_iterator->first.as<string>();

                        if (first_key == second_key) {
                            existing_map[existing_map_iterator->first] = desired_value_iterator->second;
                        }
                    }
                }
            }
        }
    }
}

// make sure that all variables matked as NOFLT are set
bool GOption::does_the_option_set_all_necessary_values(YAML::Node v) {
    vector <string> this_keys;

    // currently we only have maps in the sequence
    switch (v.Type()) {
        case YAML::NodeType::Map:
            for (YAML::const_iterator it = v.begin(); it != v.end(); ++it) {
                this_keys.push_back(it->first.as<string>());
            }
            break;
        default:
            break;
    }

    bool it_does = false;
    for (auto key: mandatory_keys) {
        if (find(this_keys.begin(), this_keys.end(), key) != this_keys.end()) {
            it_does = true;
        } else {
            it_does = false;
            break;
        }
    }

    return it_does;
}

// print the option
void GOption::save_option(ofstream *yaml_conf) {

    // setting style to block
    // this does not work with command line passed values
    value.SetStyle(YAML::EmitterStyle::Block);

    *yaml_conf << value << endl;
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

        for (unsigned i = 0; i < (unsigned) yvalues.size(); i++) {
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
