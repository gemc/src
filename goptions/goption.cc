// goptions
#include "goption.h"
#include "goptionsConventions.h"

// gemc
#include "gutilities.h"

// c++
#include <iostream>

using namespace std;

// sets the value to the scalar option
void GOption::set_scalar_value(string v) {
    string value_to_set = gutilities::replaceCharInStringWithChars(v, ",", "");

    auto scalar = values.front();
    string key = scalar.begin()->first.as<string>();
    scalar[key] = value_to_set;
}


// sets the value of the option based on the parsed yaml node
void GOption::set_value(YAML::Node v) {
    // if the option is cumulative,
    if (isCumulative) {
        YAML::Node to_set;
        // sequence of maps
        bool all_good = true;
        for (auto element: v) {
            if (!does_the_option_set_all_necessary_values(element)) {
                all_good = false;
                cerr << FATALERRORL << "Trying to set " << YELLOWHHL << name << RSTHHR << " but missing mandatory values." << endl;
                cerr << "        Use the option: " << YELLOWHHL << " help " << name << " " << RSTHHR << " for details." << endl << endl;
                exit(EC__MANDATORY_NOT_FILLED);
            }
        }
        if (all_good) {
            values.push_back(v);
        }

    } else {
        // if the option is not cumulative, the value
        // is already set to the default value.
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
